<?php

// $Id$

/**
 * @file
 * Ding user plugin.
 */

$plugin = array(
  'description' => t('ALMA user provider'),
  // Version compatibility.
  'version' => 1,
);

/**
 * Get information on a user.
 *
 * Returns an array of user data, which as a minimum should contain
 * the keys 'user_id' and 'user_name', which is used to bind to a
 * Drupal user.
 *
 * @param object $account
 *   User object.
 * @param boolean $extended
 *   Optional, whether to return extended information.
 *
 * @return array
 *   An array of user information.
 */
function alma_user_user_get_info($account, $extended = FALSE) {
  $creds = _alma_user_get_credentials($account);
  if ($creds == DING_PROVIDER_AUTH_REQUIRED) {
    return $creds;
  }
  return alma_client_get_patron_info($creds['user_id'], $creds['password'], $extended);
}

/**
 * Update user information.
 *
 * Update the user information with the given values. Only set keys
 * should be considered, with NULL values signalling deletion.
 *
 * @param object $account
 *   Object of user to be updated.
 * @param array $new_info
 *   New user information.
 *
 * @return void
 */
function alma_user_user_update_info($account, $new_info) {
  return;
}

/**
 * Returns whether the account is currently 'logged in' to the user
 * backend.
 *
 * If not, we'll need to authenticate before we can do anything.
 *
 * @param object $account
 *   The user to check.
 *
 * @return boolean
 *   TRUE if the account is authenticated.
 */
function alma_user_user_logged_in($account) {
  $creds = _alma_user_get_credentials($account);
  return $creds != DING_PROVIDER_AUTH_REQUIRED;
}

/**
 * Form callback for library settings.
 *
 * This is a regular form callback.
 */
function alma_user_user_library_settings_form(&$form_state, $account) {
  $form['account'] = array(
    '#type' => 'value',
    '#value' => $account,
  );

  $form['preferred_branch'] = array(
    '#type'          => 'select',
    '#title'         => t('Preferred library'),
    '#options'       => alma_client()->get_reservation_branches(),
    '#default_value' => $account->preferred_branch,
  );

  $form['interest_period'] = array(
    '#type'        => 'select',
    '#title'       => t('Period of interest'),
    '#description' => t('How long are you willing to wait for a reservation?'),
    '#options'     => array(
      '30'         => t('A month'),
      '60'         => t('Two months'),
      '90'         => t('Three months'),
      '183'        => t('Six months'),
      '365'        => t('A year'),
    ),
    '#default_value' => (isset($account->interest_period) && $account->interest_period > 10) ? $account->interest_period : ALMA_USER_DEFAULT_INTEREST_PERIOD,
  );

  $form['suspend_start'] = array(
    '#type'          => 'date_popup',
    '#title'         => t('Suspend reservations from'),
    '#date_format'   => DATE_FORMAT_DATE,
    '#default_value' => (isset($account->user_info['absent_periods'][0])) ? $account->user_info['absent_periods'][0]['from_date'] : '',
  );

  $form['suspend_end'] = array(
    '#type'          => 'date_popup',
    '#title'         => t('To'),
    '#date_format'   => DATE_FORMAT_DATE,
    '#default_value' => (isset($account->user_info['absent_periods'][0])) ? $account->user_info['absent_periods'][0]['to_date'] : '',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#weight' => 50,
  );

  return $form;
}

/**
 * Validation for user library settings.
 */
function alma_user_user_library_settings_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['suspend_start']) && !empty($form_state['values']['suspend_end'])) {
    $start = date_make_date($form_state['values']['suspend_start']);
    $end = date_make_date($form_state['values']['suspend_end']);

    if ($start <= new DateTime()) {
      form_set_error('suspend_start', t('Start date must later than today'));
    }

    if ($start >= $end) {
      form_set_error('suspend_start', t('Start date must be before end date'));
    }
    else {
      $form_state['values']['suspend'] = array('start' => $start, 'end' => $end);
    }
  }
}

/**
 * Submit handler for user library settings.
 */
function alma_user_user_library_settings_form_submit($form, &$form_state) {
  $creds = _alma_user_get_credentials($form_state['values']['account']);

  $info = alma_client_get_patron_info($creds['user_id'], $creds['password'], TRUE);

  if (isset($form_state['values']['suspend'])) {
    $start = date_format_date($form_state['values']['suspend']['start'], 'custom', DATE_FORMAT_DATE);
    $end = date_format_date($form_state['values']['suspend']['end'], 'custom', DATE_FORMAT_DATE);
  }

  if (isset($info['absent_periods']) && !empty($info['absent_periods'])) {
    if (isset($start) && isset($end)) {
      // Update the current period.
      alma_client_change_absent_period($creds['user_id'], $creds['password'], $info['absent_periods'][0]['id'], $start, $end);
    }
    else {
      // If either date field had no valid value, remove the absent period.
      alma_client_remove_absent_period($creds['user_id'], $creds['password'], $info['absent_periods'][0]['id']);
    }
  }
  elseif (isset($start) && isset($end)) {
    // Create a new absent period.
    alma_client_add_absent_period($creds['user_id'], $creds['password'], $start, $end);
  }

  if ($form_state['values']['preferred_branch'] != $form_state['values']['account']->preferred_branch) {
    // Update the preferred branch in the database.
    db_query("UPDATE {alma_user} SET preferred_branch = '%s' WHERE uid = %d LIMIT 1;", $form_state['values']['preferred_branch'], $form_state['values']['account']->uid);

    // And save the new choice to Alma.
    alma_client_change_patron_preferences($creds['user_id'], $creds['password'], $form_state['values']['preferred_branch']);
  }
  if ($form_state['values']['interest_period'] != $form_state['values']['account']->interest_period) {
    db_query('UPDATE {alma_user} SET interest_period = %d WHERE alma_id = "%s" LIMIT 1;', $form_state['values']['interest_period'], $form_state['values']['account']->library_user_id);
  }
}
