<?php
// $Id$

/**
 * AHAH callback to get library item availability.
 *
 * @param string $input
 *    Input string, one or more Alma item IDs, comma-separated.
 */
function alma_item_availability_page($input) {
  $data = alma_client_get_availability($input);

  // Make sure we return an array if something goes wrong.
  if (!is_array($data)) {
    $data = array();
  }

  // We are returning JavaScript, so tell the browser.
  drupal_set_header('Content-Type: application/json; charset=utf-8');
  echo json_encode($data);
  return;
}

/**
 * AHAH callback to get library item details.
 *
 * @param string $input
 *    Input string, one or more Alma item IDs, comma-separated.
 */
function alma_item_details_page($input) {
  $details = alma_client_catalogue_record_detail($input);
  $items = (!empty($details['records'])) ? $details['records'] : array();

  // We are returning JavaScript, so tell the browser.
  drupal_set_header('Content-Type: application/json; charset=utf-8');
  echo json_encode($items);
  return;
}

