<?php
// $Id$

/**
 * Alma authentication form.
 *
 * Only used when a logged in user requires access to data from Alma and
 * his Alma credentials are not cached.
 *
 * @param array $form_state
 *    Drupal's form state.
 * @param stdClass $account
 *    User account object.
 * @param mixed $explanation
 *    Explanation for the login form. NULL for the default message.
 * @param mixed $redirect
 *    Redirect destination – boolean TRUE for default destination
 *    (user's profile page).
 */
function alma_user_forms_alma_authenticate($form_state, $account, $explanation=NULL, $redirect=TRUE) {
  $form = array();

  $form['redirect'] = array(
    '#type' => 'value',
    '#value' => $redirect,
  );

  // Strict NULL check to allow an empty explanation string.
  if (is_null($explanation)) {
    $explanation = t('To access your personal information, you need to authenticate.');
  }

  $form['explanation'] = array(
    '#value' => $explanation,
  );

  $form['account'] = array(
    '#type' => 'value',
    '#value' => $account,
  );

  $form['borr_card'] = array(
    '#type' => 'password',
    '#title' => t('Borrower card'),
    '#required' => TRUE,
  );

  $form['pin_code'] = array(
    '#type' => 'password',
    '#title' => t('PIN code'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Log in'),
  );

  return $form;
}

/**
 * Validation for authentication form.
 */
function alma_user_forms_alma_authenticate_validate($form, &$form_state) {
  // Remove everything but numbers from the borr_card field value.
  $card = preg_replace('/\D+/', '', $form_state['values']['borr_card']);

  // Lookup user data
  $data = alma_client_get_patron_info($card, $form_state['values']['pin_code']);

  // If we got no data or account IDs do not match, it usually means that the
  // user supplied the wrong credentials.
  if (!$data || $data['patron_id'] != $form_state['values']['account']->alma_id) {
    form_set_error('borr_card', t('Invalid borrower card or pin code'));
  }
  else {
    // Validation succeeded, so cache the correct credentials.
    alma_user_cache_credentials($card, $form_state['values']['borr_card'], $form_state['values']['pin_code']);
  }
}

/**
 * Submit handler for authentication form.
 */
function alma_user_forms_alma_authenticate_submit($form, &$form_state) {
  // If we do not have a destination already, use the redirect value.
  if (!isset($_REQUEST['destination']) && $form_state['values']['redirect']) {
    // If redirect is boolean TRUE, use the default redirect.
    if (is_bool($form_state['values']['redirect'])) {
      $form_state['redirect'] = 'user/' . $form_state['values']['account']->uid . '/profile';
    }
    else {
      // Otherwise, assume that the redirect value is a sane destination.
      $form_state['redirect'] = $form_state['values']['redirect'];
    }
  }
}

/**
 * Change PIN code form.
 */
function alma_user_forms_change_pin($form_state, $account) {
  $pin_length = variable_get('alma_user_pin_length', 4);
  $form = array();

  $form['account'] = array(
    '#type' => 'value',
    '#value' => $account,
  );

  $form['current_pin'] = array(
    '#type' => 'password',
    '#title' => t('Current PIN code'),
    '#required' => TRUE,
    '#maxlength' => $pin_length,
    '#size' => $pin_length,
  );

  $form['new_pin'] = array(
    '#type' => 'password',
    '#title' => t('New PIN code'),
    '#required' => TRUE,
    '#maxlength' => $pin_length,
    '#size' => $pin_length,
  );

  $form['new_pin_confirm'] = array(
    '#type' => 'password',
    '#title' => t('Repeat new PIN code'),
    '#required' => TRUE,
    '#maxlength' => $pin_length,
    '#size' => $pin_length,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save new PIN code'),
  );

  return $form;
}

/**
 * Validation for PIN code changing form.
 */
function alma_user_forms_change_pin_validate($form, &$form_state) {
  $pin_length = variable_get('alma_user_pin_length', 4);
  $creds = alma_user_get_credentials($form_state['values']['account']);
  if ($form_state['values']['current_pin'] != $creds['pin_code']) {
    form_set_error('current_pin', t('Please provide your current PIN code.'));
  }

  if ($form_state['values']['current_pin'] == $form_state['values']['new_pin']) {
    form_set_error('new_pin', t('The new PIN code must be different than the current PIN code.'));
  }

  if ($form_state['values']['new_pin_confirm'] != $form_state['values']['new_pin']) {
    form_set_error('new_pin_confirm', t('Please input your new PIN code twice for confirmation.'));
  }

  if (!ctype_digit($form_state['values']['new_pin']) || strlen($form_state['values']['new_pin']) != $pin_length) {
    form_set_error('new_pin', t('The new PIN code must be @num numerical digits.', array('@num' => $pin_length)));
  }
}

/**
 * Submit handler for the PIN code changing form.
 */
function alma_user_forms_change_pin_submit($form, &$form_state) {
  $creds = alma_user_get_credentials($form_state['values']['account']);
  alma_client_change_pin($creds['borr_card'], $creds['pin_code'], $form_state['values']['new_pin']);

  // Cache the new pin-code
  alma_user_cache_credentials($form_state['values']['account'], $creds['borr_card'], $form_state['values']['new_pin']);

  // If we do not have a destination already, go to the profile.
  if (!isset($_REQUEST['destination'])) {
    $form_state['redirect'] = 'user/' . $form_state['values']['account']->uid . '/profile';
  }
}

/**
 * User library settings form.
 */
function alma_user_forms_library_settings($form_state, $account) {
  $creds = alma_user_get_credentials($account);
  $info = alma_client_get_patron_info($creds['borr_card'], $creds['pin_code'], TRUE);
  $account->alma_info = $info;
  $form = array();

  $form['account'] = array(
    '#type' => 'value',
    '#value' => $account,
  );

  $form['preferred_branch'] = array(
    '#type' => 'select',
    '#title' => t('Preferred library'),
    '#options' => alma_client_get_reservation_branches(),
    '#default_value' => $account->preferred_branch,
  );

  $form['interest_period'] = array(
    '#type' => 'select',
    '#title' => t('Period of interest'),
    '#description' => t('How long are you willing to wait for a reservation?'),
    '#options' => array(
      '30' => t('A month'),
      '60' => t('Two months'),
      '90' => t('Three months'),
      '183' => t('Six months'),
      '365' => t('A year'),
    ),
    '#default_value' => (isset($account->interest_period) && $account->interest_period > 10) ? $account->interest_period : ALMA_USER_DEFAULT_INTEREST_PERIOD,
  );

  $form['suspend_start'] = array(
    '#type' => 'date_popup',
    '#title' => t('Suspend reservations from'),
    '#date_format' => DATE_FORMAT_DATE,
    '#default_value' => (isset($info['absent_periods'][0])) ? $info['absent_periods'][0]['from_date'] : '',
  );

  $form['suspend_end'] = array(
    '#type' => 'date_popup',
    '#title' => t('To'),
    '#date_format' => DATE_FORMAT_DATE,
    '#default_value' => (isset($info['absent_periods'][0])) ? $info['absent_periods'][0]['to_date'] : '',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );

  return $form;
}

/**
 * Validation for user library settings.
 */
function alma_user_forms_library_settings_validate($form, &$form_state) {
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
function alma_user_forms_library_settings_submit($form, &$form_state) {
  $creds = alma_user_get_credentials($form_state['values']['account']);

  $info = alma_client_get_patron_info($creds['borr_card'], $creds['pin_code'], TRUE);

  if (isset($form_state['values']['suspend'])) {
    $start = date_format_date($form_state['values']['suspend']['start'], 'custom', DATE_FORMAT_DATE);
    $end = date_format_date($form_state['values']['suspend']['end'], 'custom', DATE_FORMAT_DATE);
  }

  if (isset($info['absent_periods']) && !empty($info['absent_periods'])) {
    if (isset($start) && isset($end)) {
      // Update the current period.
      alma_client_change_absent_period($creds['borr_card'], $creds['pin_code'], $info['absent_periods'][0]['id'], $start, $end);
    }
    else {
      // If either date field had no valid value, remove the absent period.
      alma_client_remove_absent_period($creds['borr_card'], $creds['pin_code'], $info['absent_periods'][0]['id']);
    }
  }
  elseif (isset($start) && isset($end)) {
    // Create a new absent period.
    alma_client_add_absent_period($creds['borr_card'], $creds['pin_code'], $start, $end);
  }

  if ($form_state['values']['preferred_branch'] != $form_state['values']['account']->preferred_branch) {
    // Update the preferred branch in the database.
    db_query("UPDATE {alma_user} SET preferred_branch = '%s' WHERE uid = %d LIMIT 1;", $form_state['values']['preferred_branch'], $form_state['values']['account']->uid);

    // And save the new choice to Alma.
    alma_client_change_patron_preferences($creds['borr_card'], $creds['pin_code'], $form_state['values']['preferred_branch']);
  }

  if ($form_state['values']['interest_period'] != $form_state['values']['account']->interest_period) {
    db_query('UPDATE {alma_user} SET interest_period = %d WHERE uid = %d LIMIT 1;', $form_state['values']['interest_period'], $form_state['values']['account']->uid);
  }
}

/**
 * Reservation details form.
 */
function alma_user_forms_reservation_details($form_state, $account, $records) {
  $cancel_fetchable = (boolean) variable_get('alma_user_cancel_fetchable_reservable', TRUE);
  $form = array();

  $form['account'] = array(
    '#type' => 'value',
    '#value' => $account,
  );

  $form['reservation_data'] = array(
    '#type' => 'value',
    '#grouped' => array(),
    '#value' => array(),
  );

  $form['reservations'] = array(
    '#tree' => TRUE,
  );

  foreach ($records as $reservation) {
    $form['reservation_data']['#value'][$reservation['id']] = $reservation;
    $form['reservation_data']['#grouped'][$reservation['status']][$reservation['id']] = $reservation;

    if ($reservation['status'] == 'active' || ($cancel_fetchable && $reservation['status'] == 'fetchable')) {
      $form['reservations'][$reservation['id']] = array(
        '#type' => 'checkbox',
      );
    }
  }

  $form['remove'] = array(
    '#type' => 'submit',
    '#value' => t('Delete selected reservations'),
    '#attributes' => array('class' => 'cancel-button'),
  );

  $form['new_expiry'] = array(
    '#type' => 'date_popup',
    '#title' => t('Valid to'),
    '#date_format' => DATE_FORMAT_DATE,
    '#date_year_range' => '-0:+2',
  );

  $form['new_pickup_branch'] = array(
    '#type' => 'select',
    '#title' => t('New pickup branch'),
    '#options' => alma_client_get_reservation_branches(),
    '#default_value' => $account->preferred_branch,
  );

  $form['update'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes for selected reservations'),
    '#attributes' => array('class' => 'update-button'),
  );

  return $form;
}

/**
 * Reservation details form validation.
 */
function alma_user_forms_reservation_details_validate($form, &$form_state) {
	$data = $form_state['values'];

	// Prevent users from updating reservations which is ready for pickup at a branch
	if (strpos($form_state['clicked_button']['#attributes']['class'], 'update-button') !== FALSE) {
	  $branches = alma_client_get_branches();
		foreach ($data['reservations'] as $id => $selected) {
	    if ($selected == 1) {
	    	$reservation = $data['reservation_data'][$id]; 
	      if (($reservation['status'] == 'fetchable') && // Detect fetable reservations...
	          (($reservation['pickup_branch'] != $data['new_pickup_branch']) || // where the user tried to alter pickup branch
	          $data['new_expiry'])) { // or alter expiry date
	        $response = alma_client_catalogue_record_detail($reservation['record_id']);
	        if ($response['request_status'] == 'ok' && ($record = array_shift($response['records']))) {
	        	form_set_error('reservations]['.$id, t('You can not perform changes to reservation of !title as the item is ready for pick up at !branch', array('!title' => $record['titles'][0], '!branch' => $branches[$reservation['pickup_branch']])));
	        } else {
	          form_set_error('reservations]['.$id, t('You can not perform changes to reservations which are ready for pick up'));
	        }
	      }
	    }
	  }
	}
  
	if (!empty($data['new_expiry'])) {
    list($year, $month, $day) = explode('-', trim($data['new_expiry']));
    if (!checkdate($month, $day, $year)) {
      form_set_error('new_expiry', t('Invalid date'));
    }

    $timestamp = strtotime($data['new_expiry']);

    if ($timestamp <= $_SERVER['REQUEST_TIME']) {
      form_set_error('new_expiry', t('Date must be later than today.'));
    }
    else {
      form_set_value($form['new_expiry'], format_date($timestamp, 'custom', 'Y-m-d'), $form_state);
    }
    
  }
}

/**
 * Reservation details form submit handler.
 */
function alma_user_forms_reservation_details_submit($form, &$form_state) {
  $selected = array();
  $reservations = $form_state['values']['reservation_data'];

  foreach ($form_state['values']['reservations'] as $id => $val) {
    if ($val == 1) {
      $selected[] = $id;
    }
  }

  switch ($form_state['clicked_button']['#attributes']['class']) {
    case 'update-button':
      $changes = array();
      if (!empty($form_state['values']['new_expiry'])) {
        $changes['valid_to'] = $form_state['values']['new_expiry'];
      }

      if (!empty($form_state['values']['new_pickup_branch'])) {
        $changes['pickup_branch'] = $form_state['values']['new_pickup_branch'];
      }

      if (!empty($selected) && !empty($changes)) {
        $creds = alma_user_get_credentials($form_state['values']['account']);
        foreach ($selected as $id) {
          alma_client_change_reservation($creds['borr_card'], $creds['pin_code'], $reservations[$id], $changes);
        }
      }
      break;
      
    case 'cancel-button':
      $creds = alma_user_get_credentials($form_state['values']['account']);
      foreach ($selected as $id) {
        alma_client_remove_reservation($creds['borr_card'], $creds['pin_code'], $reservations[$id]);
      }
      break;
  }
}

/**
 * Loan details form.
 */
function alma_user_forms_loan_details($form_state, $account, $records) {
  $grouped = _alma_user_forms_group_loans($records);
  $renew_buttons = FALSE;
  $form = array();

  $form['account'] = array(
    '#type' => 'value',
    '#value' => $account,
  );

  $form['loan_data'] = array(
    '#type' => 'value',
    '#grouped' => $grouped,
    '#value' => $records,
  );

  $form['loans'] = array(
    '#tree' => TRUE,
  );

  foreach ($records as $loan) {
    if ($loan['is_renewable']) {
      $renew_buttons = TRUE;
      $form['loans'][$loan['id']] = array(
        '#type' => 'checkbox',
      );
    }
  }


  if ($renew_buttons) {
    $form['renew'] = array(
      '#type' => 'submit',
      '#value' => t('Renew selected'),
    );

    $form['renew_all'] = array(
      '#type' => 'submit',
      '#value' => t('Renew all'),
      '#attributes' => array('class' => 'renew-all-button'),
    );
  }

  return $form;
}

/**
 * Loan details submit handler.
 */
function alma_user_forms_loan_details_submit($form, $form_state) {
  $selected = array();

  switch ($form_state['clicked_button']['#parents'][0]) {
    case 'renew':
      foreach ($form_state['values']['loans'] as $key => $value) {
        if ($value) {
          $selected[] = $key;
        }
      }
      break;
    case 'renew_all':
      foreach ($form_state['values']['loan_data'] as $loan) {
        if ($loan['is_renewable']) {
          $selected[] = $loan['id'];
        }
      }
      break;
  }

  $creds = alma_user_get_credentials($form_state['values']['account']);
  alma_client_renew_loan($creds['borr_card'], $creds['pin_code'], $selected);
}

/**
 * Helper function to group loans by their due dates.
 */
function _alma_user_forms_group_loans($records) {
  $groups = array();
  $today = date('Y-m-d', $_SERVER['REQUEST_TIME']);

  foreach ($records as $loan) {
    if ($loan['due_date'] < $today) {
      $groups['overdue'][$loan['id']] = $loan['id'];
    }
    elseif ($loan['due_date'] == $today) {
      $groups['due'][$loan['id']] = $loan['id'];
    }
    else {
      $groups[$loan['due_date']][$loan['id']] = $loan['id'];
    }
  }
  return $groups;
}

/**
 * Debts details form.
 */
function alma_user_forms_debts_details(&$form_state, $account, $debt_data) {
  $form = array();

  $form['account'] = array(
    '#type' => 'value',
    '#value' => $account,
  );

  $form['debt_data'] = array(
    '#type' => 'value',
    '#value' => $debt_data,
  );

  $form['debts'] = array(
    '#tree' => TRUE,
  );

  foreach ($debt_data['debts'] as $debt) {
    $form['debts'][$debt['id']] = array(
      '#type' => 'checkbox',
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Pay now'),
  );

  return $form;
}

/**
 * Validation for debts details form.
 */
function alma_user_forms_debts_details_validate($form, &$form_state) {
  $selected_debts = array_keys(array_filter($form_state['values']['debts']));

  if (empty($selected_debts)) {
    form_set_error('debts', t('Please select one or more debts to pay.'));
  }
  $form_state['values']['selected_debts'] = $selected_debts;

  $total = 0.0;

  foreach ($selected_debts as $debt_id) {
    $total += (float) $form_state['values']['debt_data']['debts'][$debt_id]['amount'];
  }

  if ($total <= 0.0) {
    form_set_error('debts', t('Payment must be larger than zero.'));
  }
  $form_state['values']['total_amount'] = $total;
}

