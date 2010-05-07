<?php
// $Id$

/**
 * Alma user settings form.
 */
function alma_user_admin_settings_form(&$form_state) {
  $form = array();

  $form['alma_user_credential_cache_duration'] = array(
    '#type' => 'select',
    '#title' => t('Credential cache duration'),
    '#description' => t('Determines how long the user’s Alma credentials will be cached for. For the duration of the cache, the user will be able to access his personal information and interact with the library system without having to provide his credentials again.'),
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

  $form['alma_user_pin_length'] = array(
    '#type' => 'textfield',
    '#title' => t('PIN code length'),
    '#default_value' => variable_get('alma_user_pin_length', 4),
    '#required' => TRUE,
    '#size' => 4,
  );

  return system_settings_form($form);
}

/**
 * Validation for settings form.
 */
function alma_user_admin_settings_form_validate($form, &$form_state) {
  // Force PIN length to be an integer.
  $form_state['values']['alma_user_pin_length'] = intval($form_state['values']['alma_user_pin_length']);

  if ($form_state['values']['alma_user_pin_length'] <= 0) {
    form_set_error('alma_user_pin_length', t('PIN code length must be a number larger than zero.'));
  }
}

