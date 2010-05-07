<?php
// $Id$

/**
 * @file relevant.inc
 * Provides several library portal page related panels content types.
 */

/**
 * Callback function to supply a list of content types.
 *
 * This is only required to make CTools know that we're providing 
 * multiple types here (since we have no single => TRUE in the array).
 */
function alma_user_profile_ctools_content_types() {
  return array(
    'title' => t('Library patron profile'),
  );
}

/**
 * Callback function to supply a list of content types.
 */
function alma_user_profile_content_type_content_types() {
  $types = array(
    'alma_user_personal_info' => array(
      'title' => t('Personal information'),
      'description' => t('Display of the patron’s personal information, with options to edit them.'),
    ),
    'alma_user_library_settings' => array(
      'title' => t('Library settings'),
      'description' => t('The patron’s library settings. Allows the patron to choose a preferred library and suspend his reservations for a period.'),
    ),
    'alma_user_reservations_list' => array(
      'title' => t('Reservation list'),
      'description' => t('A simple list of the patron’s reservations.'),
    ),
    'alma_user_reservations_details' => array(
      'title' => t('Reservation details'),
      'description' => t('A detailed list of the patron’s reservations.'),
    ),
    'alma_user_loans_list' => array(
      'title' => t('Loan list'),
      'description' => t('A simple list of the patron’s loans.'),
    ),
    'alma_user_loans_details' => array(
      'title' => t('Loan details'),
      'description' => t('A detailed list of the patron’s loans.'),
    ),
    'alma_user_debts_notice' => array(
      'title' => t('User debts notice'),
      'description' => t('Provides a short noice to the user about his library debts.'),
    ),
    'alma_user_debts_details' => array(
      'title' => t('User debts details'),
      'description' => t('Provides the full list of user debts, with an option to pay them.'),
    ),
  );

  // Append our default settings to every type.
  foreach ($types as $name => $type) {
    $types[$name] += array(
      'category' => t('Library patron profile'),
      'required context' => new ctools_context_required(t('Patron'), 'user'),
    );
  }
  
  return $types;
}

/**
 * Return the pane contents.
 */
function alma_user_profile_content_type_render($subtype, $conf, $panel_args, $context) {
  $account = isset($context->data) ? drupal_clone($context->data) : NULL;
  $alma_creds = alma_user_get_credentials($account); 
  $block = new stdClass();

  if ($alma_creds) {
    switch ($subtype) {
      case 'alma_user_personal_info':
        $data = alma_client_get_patron_info($alma_creds['borr_card'], $alma_creds['pin_code']);
        $block->title = t('Your profile');
        $block->content = theme('alma_user_personal_info', $account, $data);
        break;

      case 'alma_user_library_settings':
        module_load_include('forms.inc', 'alma_user');
        $block->title = t('Your library');
        $block->content = drupal_get_form('alma_user_forms_library_settings', $account);
        break;

      case 'alma_user_reservations_list':
        $list = alma_client_get_reservations($alma_creds['borr_card'], $alma_creds['pin_code']);
        $block->title = t('Your reservations');
        $block->content = theme('alma_user_record_list', $list);
        break;

      case 'alma_user_reservations_details':
        module_load_include('forms.inc', 'alma_user');
        $list = alma_client_get_reservations($alma_creds['borr_card'], $alma_creds['pin_code']);
        $block->title = t('Your reservations');
        $block->content = drupal_get_form('alma_user_forms_reservation_details', $account, $list);
        break;

      case 'alma_user_loans_list':
        $list = alma_client_get_loans($alma_creds['borr_card'], $alma_creds['pin_code']);
        $block->title = t('Your loans');
        $block->content = theme('alma_user_record_list', $list);
        break;
      case 'alma_user_loans_details':
        module_load_include('forms.inc', 'alma_user');
        $list = alma_client_get_loans($alma_creds['borr_card'], $alma_creds['pin_code']);
        $block->title = t('Your loans');
        $block->content = drupal_get_form('alma_user_forms_loan_details', $account, $list);
        break;

      case 'alma_user_debts_notice':
        $data = alma_client_get_debts($alma_creds['borr_card'], $alma_creds['pin_code']);
        if (count($data['debts']) == 1) {
          $block->content = t('You owe @amount for “%note”. !pay_link.', array(
            '@amount' => $data['total_formatted'],
            '%note' => trim($data['debts'][0]['note']),
            '!pay_link' => l(t('Pay now'), 'user/' . $account->uid . '/debts'),
          ));
        }
        elseif (count($data['debts']) > 1) {
          $block->content = t('You owe @amount for “%note” and !others_link. !pay_link.', array(
            '@amount' => $data['total_formatted'],
            '%note' => trim($data['debts'][0]['note']),
            '!others_link' => l(format_plural(count($data['debts']) - 1, 'one other', '@count others'),
                                'user/' . $account->uid . '/debts'),
            '!pay_link' => l(t('Pay now'), 'user/' . $account->uid . '/debts'),
          ));
        }
        break;

      case 'alma_user_debts_details':
        module_load_include('forms.inc', 'alma_user');
        $data = alma_client_get_debts($alma_creds['borr_card'], $alma_creds['pin_code']);
        $block->content = drupal_get_form('alma_user_forms_debts_details', $account, $data);
        break;
    }
  }
  elseif ($subtype == 'alma_user_personal_info') {
    $block->content = t('Error fetching your library patron profile.');
  }

  return $block;
}

/**
 * Return the pane titles.
 */
function alma_user_profile_content_type_admin_title($subtype, $conf, $context) {
  switch($subtype) {
    case 'alma_user_personal_info':
      return t('“@s”: Personal information', array('@s' => $context->identifier));
    case 'alma_user_library_settings':
      return t('“@s”: Library settings', array('@s' => $context->identifier));
    case 'alma_user_reservations_list':
      return t('“@s”: Reservation list', array('@s' => $context->identifier));
    case 'alma_user_reservations_details':
      return t('“@s”: Reservation details', array('@s' => $context->identifier));
    case 'alma_user_loans_list':
      return t('“@s”: Loan list', array('@s' => $context->identifier));
    case 'alma_user_loans_details':
      return t('“@s”: Loan details', array('@s' => $context->identifier));
    case 'alma_user_debts_notice':
      return t('“@s”: Debts notice', array('@s' => $context->identifier));
    case 'alma_user_debts_details':
      return t('“@s”: Debts list', array('@s' => $context->identifier));
  }
}

/**
 * Returns an edit form for the content type.
 *
 * We're not actually doing anything here, but we need to have this hook 
 * for this content type to work.
 */
function alma_user_profile_content_type_edit_form(&$form, &$form_state) {
  return $form;
}

