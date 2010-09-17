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
  // Get creds..
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
 * @param boolean $redirect
 *   Whether it's OK to redirect to log the user in.
 *
 * @return boolean
 *   TRUE if the account is authenticated.
 */
function alma_user_user_logged_in($account, $redirect = FALSE) {
  return (bool) _alma_user_get_credentials($account, $redirect);
}
