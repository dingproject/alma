<?php
// $Id$

/**
 * @file alma.admin.inc
 * Admin settings for the Alma Drupal module.
 */

/**
 * Main settings form implementation.
 */
function alma_admin_settings_form($form_state) {
  $form = array();

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
  
  return system_settings_form($form);
}

