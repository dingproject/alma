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
  $creds = ding_library_user_get_credentials($account);
  return $creds != DING_PROVIDER_AUTH_REQUIRED;
}

/**
 *
 */
function alma_user_user_authenticate($name, $pass) {
  $return = array(
    'result' => FALSE,
  );
  $info = alma_client_invoke('get_patron_info', $name, $pass, TRUE);
  if (!$info) {
    // Not found.
    return;
  }

  if (isset($info['blocks']) && !empty($info['blocks'])) {
    $block_codes = alma_user_block_codes();
    foreach ($user_info['blocks'] as $block) {
      $return['messages'][] = array(t('Your account is blocked: %message.', array('%message' => $block_codes[$block['code']])), 'warning');
    }
  }

  $return['result'] = TRUE;
  $return['authname'] = md5(drupal_get_private_key() . $info['user_id']) . '@alma_user';
  $return['creds'] = array(
    'user_id' => $name,
    'password' => $pass,
  );

  $return['user'] = array(
    'display_name' => $info['user_name'],
    'auth_mail' => '',
  );


  if (isset($info['mails'][0]) && valid_email_address($info['mails'][0]['mail'])) {
    $return['user']['auth_mail'] = $info['mails'][0]['mail'];
  }

  return $return;
}

/**
 * Form callback for plugin settings.
 *
 * This is a regular form callback.
 */
function alma_user_user_settings_form() {
  $form['alma_base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Alma base URL'),
    '#description' => t('The base URL for constructing request to the Alma REST service, usually something like https://server.example.com:8000/alma/ or http://10.0.0.34:8010/alma/'),
    '#required' => TRUE,
    '#default_value' => variable_get('alma_base_url', ''),
  );

  $form['alma_enable_logging'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable logging'),
    '#default_value' => variable_get('alma_enable_logging', ''),
    '#description' => t('Logs requests to the Alma webservice. Sensitive information such as CPR number and PIN code is stripped from the requests'),
  );

  $form['alma_user_credential_cache_duration'] = array(
    '#type' => 'select',
    '#title' => t('Credential cache duration'),
    '#description' => t('Determines how long the user’s library system credentials will be cached for. For the duration of the cache, the user will be able to access his personal information and interact with the library system without having to provide his credentials again.'),
    '#options' => array(
      300 => t('5 minutes'),
      900 => t('15 minutes'),
      1800 => t('30 minutes'),
      3600 => t('1 hour'),
      10800 => t('3 hours'),
      43200 => t('12 hours'),
      86400 => t('1 day'),
      259200 => t('3 days'),
      604800 => t('7 days'),
    ),
    '#default_value' => variable_get('alma_user_credential_cache_duration', 1800),
  );

  return system_settings_form($form);
}

/**
 * Form callback for user library settings.
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
    '#options'       => alma_client_invoke('get_reservation_branches'),
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
  $creds = ding_library_user_get_credentials($form_state['values']['account']);

  $info = alma_client_invoke('get_patron_info', $creds['user_id'], $creds['password'], TRUE);

  if (isset($form_state['values']['suspend'])) {
    $start = date_format_date($form_state['values']['suspend']['start'], 'custom', DATE_FORMAT_DATE);
    $end = date_format_date($form_state['values']['suspend']['end'], 'custom', DATE_FORMAT_DATE);
  }

  if (isset($info['absent_periods']) && !empty($info['absent_periods'])) {
    if (isset($start) && isset($end)) {
      // Update the current period.
      alma_client_invoke('change_absent_period', $creds['user_id'], $creds['password'], $info['absent_periods'][0]['id'], $start, $end);
    }
    else {
      // If either date field had no valid value, remove the absent period.
      alma_client_invoke('remove_absent_period', $creds['user_id'], $creds['password'], $info['absent_periods'][0]['id']);
    }
  }
  elseif (isset($start) && isset($end)) {
    // Create a new absent period.
    alma_client_invoke('add_absent_period', $creds['user_id'], $creds['password'], $start, $end);
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
