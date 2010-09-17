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
 * @addtogroup hooks
 * @{
 */

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
 * @} End of "addtogroup hooks".
 */
