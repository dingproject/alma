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
function alma_user_reservation_create($user, $item_id) {
  $reservation = FALSE;
  $creds       = ding_library_user_get_credentials($user);
  $has_creds   = is_array($creds);

  $interest_period = (isset($account->interest_period) && $account->interest_period > 10) ? $account->interest_period : DING_LIBRARY_USER_DEFAULT_INTEREST_PERIOD;

  if ($has_creds) {
    $params = array(
      'id'            => utf8_decode($item_id),
      'pickup_branch' => $account->preferred_branch,
      'valid_from'    => date('Y-m-d'),
      'valid_to'      => date('Y-m-d', $_SERVER['REQUEST_TIME'] + $interest_period * 86400),
    );
    $result = alma_client_invoke('add_reservation', $creds['user_id'], $creds['password'], $params);
  }

  return $result;
}


function alma_user_reservation_list($user) {
  $creds = ding_library_user_get_credentials($user, $redirect);
  $reservations = alma_client_invoke('get_reservations', $creds['user_id'], $creds['password']);
  return $reservations;
}

function alma_user_reservation_details($reservation) {
  $cache = &ctools_static(__FUNCTION__, array());
  if (!isset($cache[$reservation['id']])) {
    if ($reservation['record_id'] and $object = ting_get_object_by_local_id($reservation['record_id'])) {
      $reservation['ting_object'] = $object;
    }
    $cache[$reservation['id']] = $reservation;
  }
  // TODO: Fix up pickup_branch to something readable.
  return $cache[$reservation['id']];
}

/**
 * @return bool
 */
function alma_user_reservation_exists($user, $item_id) {
  $creds = ding_library_user_get_credentials($user);

  $reservations = alma_client_invoke('get_reservations', $borr_card, $pin_code);
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
    '#date_format' => DATE_FORMAT_DATE,
    '#date_year_range' => '-0:+2',
  );

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
    $changes['expiry'] = $options['expiry'];
  }
  if ($options['pickup_branch']) {
    $changes['pickup_branch'] = $options['pickup_branch'];
  }
  $res = array();
  foreach ($reservation_ids as $reservation_id) {
    alma_client_invoke('change_reservation', $creds['user_id'], $creds['password'], $reservations[$reservation_id], $changes);
    // AlmaClient doesn't check for success, so why should we?
    $res[$reservation_id] = TRUE;
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
