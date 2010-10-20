<?php

// $Id$

/**
 * @file
 * Ding availability plugin.
 */

$plugin = array(
  'description' => t('Alma availability plugin'),
  'version' => 1,
);

/**
 */
function alma_availability_items($provider_ids) {
  $result = alma_client_invoke('get_availability', join(',',$provider_ids));
  if (!$result) {
    $result = array();
  }
  return $result;
}

/**
 */
function alma_availability_details($provider_ids) {
  $holding_parts = array('branch', 'department', 'location', 'sublocation', 'collection');
  $details = alma_client_invoke('catalogue_record_detail', join(',',$provider_ids));

  $org = alma_get_organisation();
  $result = array();
  if ($details && isset($details['records'])) {
    foreach ($details['records'] as $alma_id => $record) {
      $holding = array(
        'local_id' => $alma_id, // Sure hope this holds.
        'available' => $record['available_count'] > 0,
        'reservable' => $record['show_reservation_button'],
        'show_reservation_button' => $record['show_reservation_button'],
        'holdings' => array(),
      );
      foreach ($record['holdings'] as $branch_holding) {
        $parts = array();
        foreach ($holding_parts as $part) {
          if (!empty($branch_holding[$part . '_id'])) {
            $parts[] = $org[$part][$branch_holding[$part . '_id']];
          }
        }
        $parts = array_filter($parts);
        if ($parts) {
          $holding['holdings'][] = join(' → ', $parts);
        }
      }
      $result[$alma_id] = $holding;
    }
  }
  return $result;
}
