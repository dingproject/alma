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
  if ($creds == DING_PROVIDER_AUTH_REQUIRED) {
    return $creds;
  }

  $loans = alma_client_get_loans($creds['user_id'], $creds['password']);
  return $loans;
}

function alma_user_loan_details($loan) {
  $cache = &ctools_static(__FUNCTION__, array());
  if (!isset($cache[$loan['id']])) {
    if ($loan['record_id'] and $object = ting_get_object_by_local_id($loan['record_id'])) {
      $loan['ting_object'] = $object;
    }
    else {
      $data = alma_client_catalogue_record_detail($loan['record_id']);
      $details = $data['records'][$loan['record_id']];
      $loan['title'] = $details['titles'][0];
      $loan['authors'] = implode(', ', $details['authors']);
      // Remote loans have negative IDs (undocumented Axiell "feature").
      if (intval($loan['id']) < 0) {
        $loan['remote_loan'] = TRUE;
      }
    }
    $cache[$loan['id']] = $loan;
  }
  return $cache[$loan['id']];
}

/**
 */
function alma_user_loan_renew_loans($account, $loan_ids) {
  $creds = _alma_user_get_credentials($user, $redirect);
  return (bool) alma_client_renew_loan($creds['user_id'], $creds['password']);

}
