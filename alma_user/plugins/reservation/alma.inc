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
