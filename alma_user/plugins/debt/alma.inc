<?php


/**
 * @file
 * Ding debt plugin.
 */

$plugin = array(
  'description' => t('Alma debt plugin'),
  'version' => 1,
);

/**
 *
 */
function alma_user_debt_settings_form() {
  $form['description'] = array(
    '#type' => 'markup',
    '#value' => t("Ding Dibs Integration module is required for payments."),
  );

  $form['status'] = array(
    '#type' => 'item',
    '#title' => t('Payments'),
    '#value' => function_exists('ding_dibs_payment_prepare') ? t('Enabled') : t('Disabled'),
  );

  return $form;
}

/**
 */
function alma_user_debt_list($account) {
  $return = array();
  $creds = ding_library_user_get_credentials($account);
  if ($creds != DING_PROVIDER_AUTH_REQUIRED) {
    $debts = alma_client_invoke('get_debts', $creds['user_id'], $creds['password']);
    if ($debts && isset($debts['debts'])) {
      foreach ($debts['debts'] as $id => $debt) {
        $return[$id] = array(
          'id' => $id,
          'display_title' => $debt['note'],
          'payable' => TRUE,
          'date' => $debt['date'],
          'amount' => $debt['amount'],
        );
      }
    }
  }
  return $return;
}
