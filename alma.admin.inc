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

  return system_settings_form($form);
}

