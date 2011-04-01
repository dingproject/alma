<?php


/**
 * @file
 * Ding branch plugin.
 */

$plugin = array(
  'description' => t('Alma branch plugin'),
  'version' => 1,
);

/**
 */
function alma_branch_list() {
  $org = alma_get_organisation(FALSE);
  return $org['branch'];
}
