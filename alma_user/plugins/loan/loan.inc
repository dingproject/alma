<?php

// $Id$

/**
 * @file
 * Ding loan plugin.
 */

$plugin = array(
  'description' => t('Alsa loan plugin'),
  'version' => 1,
);

/**
 */
function alma_user_loan_get_loans($account, $redirect = FALSE) {
  $creds = _alma_user_get_credentials($account, $redirect);
  $loans = alma_client_get_loans($creds['user_id'], $creds['password']);
  return $loans;
}

/**
 */
function alma_user_loan_renew_loan($account, $loan, $redirect = FALSE) {
  $creds = _alma_user_get_credentials($user, $redirect);
  $reservations = alma_client_get_reservations($creds['user_id'], $creds['password/email/etc']);
  return $reservations;
}
