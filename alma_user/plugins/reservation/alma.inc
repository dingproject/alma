<?php

// $Id$

/**
 * @file
 * Ding reservations plugin.
 */

$plugin = array(
  'description' => t('Alsa reservations plugin'),
  'version' => 1,
);

// ding_library_user_create_reservation($user, $item_id, $optional = FALSE)
function alma_user_reservation_create($account, $item_id, $options = array()) {
  $creds = ding_library_user_get_credentials($account);

  $interest_period = (isset($account->interest_period) && $account->interest_period > 10) ? $account->interest_period : DING_LIBRARY_USER_DEFAULT_INTEREST_PERIOD;

  if ($creds != DING_PROVIDER_AUTH_REQUIRED) {
    if (alma_user_reservation_exists($account, $item_id)) {
      return array('status' => DING_RESERVATION_ERROR_RESERVATION_EXISTS);
    }

    $params = array(
      'id'            => utf8_decode($item_id),
      'valid_from'    => date('Y-m-d'),
      'valid_to'      => isset($options['expiry']) ? alma_format_date($options['expiry']) : alma_format_date($_SERVER['REQUEST_TIME'] + ($interest_period * 86400)),
    );
    if (isset($options['pickup_branch'])) {
      $params['pickup_branch'] = $options['pickup_branch'];
    }
    else {
      if (!empty($account->preferred_branch)) {
        $params['pickup_branch'] = $account->preferred_branch;
      }
      else {
        return array('status' => DING_RESERVATION_ERROR_NO_DEFAULT_BRANCH);
      }
    }
    $res = alma_client_invoke('add_reservation', $creds['user_id'], $creds['password'], $params);
    if ($res) {
      $org = alma_get_organisation();
      $res = array(
        'status' => DING_RESERVATION_SUCCESS,
        'branch' => $org['branch'][$params['pickup_branch']],
      );
    }
    else {
      $res = array(
        'status' => DING_RESERVATION_ERROR_NOT_ALLOWED,
      );
    }
  }
  else {
    $res = array(
      'status' => $creds,
    );
  }

  return $res;
}


function alma_user_reservation_list($user) {
  $creds = ding_library_user_get_credentials($user, $redirect);
  $org_cache = &ctools_static(__FUNCTION__, array());
  if (empty($org_cache)) {
    $org_cache = alma_get_organisation();
  }
  $reservations = alma_client_invoke('get_reservations', $creds['user_id'], $creds['password']);
  foreach ($reservations as $key => &$reservation) {
    // Remote reservations have negative IDs (undocumented Axiell "feature").
    if ($reservation['id'] < 0) {
      $reservation['remote_reservation'] = TRUE;
    }
    $reservation['pickup_branch_id'] = $reservation['pickup_branch'];
    $reservation['pickup_branch'] = $org_cache['branch'][$reservation['pickup_branch']];
  }
  return $reservations;
}

function alma_user_reservation_details($reservation) {
  $cache = &ctools_static(__FUNCTION__, array());
  if (!isset($cache[$reservation['id']])) {
    if ($reservation['record_id'] and $object = ting_get_object_by_local_id($reservation['record_id'])) {
      $reservation['ting_object'] = $object;
    }
    else {
      $data = alma_client_invoke('catalogue_record_detail', $reservation['record_id']);
      $details = $data['records'][$reservation['record_id']];
      $reservation['title'] = $details['titles'][0];
      $reservation['authors'] = (is_array($details['authors'])) ? implode(', ', $details['authors']) : $details['authors'];
    }
    $cache[$reservation['id']] = $reservation;
  }
  return $cache[$reservation['id']];
}

/**
 * @return bool
 */
function alma_user_reservation_exists($user, $item_id) {
  $reservations = alma_user_reservation_list($user);
  foreach ($reservations as $res) {
    if ($res['record_id'] == $item_id) {
      return TRUE;
    }
  }

  return FALSE;
}


/**
 * Return options form fragment for updating/creating reservations.
 *
 * @param object $account
 *   User.
 * @param boolean $create
 *   Whether it's for creating or updating reservations.
 * @return array
 *   Form fragment.
 */
function alma_user_reservation_options($account, $create = TRUE) {
  $form['expiry'] = array(
    '#type' => 'date_popup',
    '#title' => t('Valid to'),
    '#date_format' => variable_get('date_format_date_short', DATE_FORMAT_DATE),
    '#date_year_range' => '-0:+2',
  );

  if ($create) {
    $interest_period = (isset($account->interest_period) && $account->interest_period > 10) ? $account->interest_period : DING_LIBRARY_USER_DEFAULT_INTEREST_PERIOD;
    $form['expiry']['#default_value'] = date('Y-m-d', $_SERVER['REQUEST_TIME'] + $interest_period * 86400);
  }

  $branches = array();
  if (!$create) {
    $branches = array(
      '' => t("No change"),
    );
  }
  $branches += alma_client_invoke('get_reservation_branches');

  $form['pickup_branch'] = array(
    '#type' => 'select',
    '#title' => t('New pickup branch'),
    '#options' => $branches,
    '#default_value' => (isset($account->preferred_branch)) ? $account->preferred_branch : NULL, 
  );

  if ($create) {
    $form['#element_validate'] = array('alma_user_element_validate_reservation_options');
  }

  return $form;
}

/**
 * Update reservations.
 *
 * @param object $account
 *   The user.
 * @param array $reservation_ids
 *   The reservations to update.
 * @param array $options
 *   The options gathered through reservation_options.
 * @return array
 *   Reservation_id => success.
 */
function alma_user_reservation_update($account, $reservation_ids, $options = array()) {
  $creds = ding_library_user_get_credentials($account);
  $reservations = alma_user_reservation_list($account);
  $changes = array();
  if ($options['expiry']) {
    $changes['valid_to'] = alma_format_date($options['expiry']);
  }
  if ($options['pickup_branch']) {
    $changes['pickup_branch'] = $options['pickup_branch'];
  }
  $res = array();
  foreach ($reservation_ids as $reservation_id) {
    $reservation = $reservations[$reservation_id];
    if (!$reservation) {
      $res[$reservation_id] = t('Internal error');
    }
    $branch_name = $reservation['pickup_branch'];
    // Revert branch name to id.
    $reservation['pickup_branch'] = $reservation['pickup_branch_id'];
    if ($reservation['status'] == 'fetchable') {
      if ((isset($changes['pickup_branch']) && ($changes['pickup_branch'] != $reservation['pickup_branch'])) || (isset($changes['valid_to']) && ($changes['valid_to'] != $reservation['valid_to']))) {

        // Get details so title is filled in.
        $reservation = alma_user_reservation_details($reservation);
        if ($reservation['ting_object']) {
          $reservation['title'] = $reservation['ting_object']->title;
        }

        if (isset($reservation['title']) and $branch_name) {
          $res[$reservation_id] = t('You can not perform changes to reservation of !title as the item is ready for pick up at !branch', array('!title' => $reservation['title'], '!branch' => $branch_name));
        }
        else {
          $res[$reservation_id] = t('You can not perform changes to reservations which are ready for pick up');
        }
      }
    }
    else {
      alma_client_invoke('change_reservation', $creds['user_id'], $creds['password'], $reservation, $changes);
      // AlmaClient doesn't check for success, so why should we?
      $res[$reservation_id] = TRUE;
    }
  }
  return $res;
}

/**
 * Delete reservations.
 *
 * @param object $account
 *   The user to delete for.
 * @param array $reservation_ids
 *   The reservations to delete.
 * @return array
 *   Reservation_id => success.
 */
function alma_user_reservation_delete($account, $reservation_ids) {
  $creds = ding_library_user_get_credentials($account);
  $reservations = alma_user_reservation_list($account);
  $res = array();
  foreach ($reservation_ids as $reservation_id) {
    alma_client_invoke('remove_reservation', $creds['user_id'], $creds['password'], $reservations[$reservation_id]);
    $res[$reservation_id] = TRUE;
  }
  return $res;
}
