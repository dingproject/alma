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

// Todo: document.

// ding_library_user_create_reservation($user, $item_id, $optional = FALSE)
function alma_user_reservation_create($user, $item_id, $optional = FALSE) {
  $reservation = false;
  $creds       = _alma_user_get_credentials($user, FALSE);
  $has_creds   = is_array($creds);

  if ($has_creds) {
    $params = array(
      'id'            => utf8_decode($item_id),
      'pickup_branch' => $optional['pickup_branch'],
      'valid_from'    => $optional['valid_from'],
      'valid_to'      => $optional['valid_to'],
    );
   $reservation = alma_client_add_reservation($creds['user_id'], $creds['password'], $params);
  }

  return $reservation;
}

// ding_library_user_update_reservation($user, $reservation, $changes)
function alma_user_reservation_update($user, $reservation, $changes) {
  $creds = _alma_user_get_credentials($user);
  alma_client_change_reservation($creds['user_id'], $creds['password'], $reservation, $changes);
}

// ding_library_user_delete_reservation($user, $reservation)
function alma_user_reservation_delete($user, $reservation) {
  $creds = _alma_user_get_credentials($user);
  alma_client()->remove_reservation($creds['user_id'], $creds['password'], $reservation);
}

// ding_library_user_get_reservations($user)
function alma_user_reservation_list($user) {
  $creds = _alma_user_get_credentials($user, $redirect);
  $reservations = alma_client_get_reservations($creds['user_id'], $creds['password']);
  return $reservations;
}

/**
 * @return bool
 */
// ding_library_user_has_reservation($user, $item_id)
function alma_user_reservation_exists($user, $item_id) {
  $creds = _alma_user_get_credentials($user);
  return alma_client_user_has_reservation($creds['user_id'], $creds['password'], $item_id);
}
