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
  $creds       = _alma_user_get_credentials($user);
  $has_creds   = is_array($creds);

  $interest_period = (isset($account->interest_period) && $account->interest_period > 10) ? $account->interest_period : DING_LIBRARY_USER_DEFAULT_INTEREST_PERIOD;

  if ($has_creds) {
    $params = array(
      'id'            => utf8_decode($item_id),
      'pickup_branch' => $account->preferred_branch,
      'valid_from'    => date('Y-m-d'),
      'valid_to'      => date('Y-m-d', $_SERVER['REQUEST_TIME'] + $interest_period * 86400),
    );
    $result = alma_client_add_reservation($creds['user_id'], $creds['password'], $params);
  }

  return $result;
}

// // ding_library_user_update_reservation($user, $reservation, $changes)
// function alma_user_reservation_update($user, $reservation, $changes) {
//   $creds = _alma_user_get_credentials($user);
//   alma_client_change_reservation($creds['user_id'], $creds['password'], $reservation, $changes);
// }

// // ding_library_user_delete_reservation($user, $reservation)
// function alma_user_reservation_delete($user, $reservation) {
//   $creds = _alma_user_get_credentials($user);
//   alma_client()->remove_reservation($creds['user_id'], $creds['password'], $reservation);
// }

// ding_library_user_get_reservations($user)
function alma_user_reservation_list($user) {
  $creds = _alma_user_get_credentials($user, $redirect);
  $reservations = alma_client_get_reservations($creds['user_id'], $creds['password']);
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
// ding_library_user_has_reservation($user, $item_id)
function alma_user_reservation_exists($user, $item_id) {
  $creds = _alma_user_get_credentials($user);
  return alma_client_user_has_reservation($creds['user_id'], $creds['password'], $item_id);
}
