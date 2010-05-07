<?php
// $Id$

/**
 * @file cart.inc
 * Provides several cart related panels content types.
 */

/**
 * Callback function to supply a list of content types.
 *
 * This is only required to make CTools know that we're providing 
 * multiple types here (since we have no single => TRUE in the array).
 */
function alma_cart_cart_ctools_content_types() {
  return array(
    'title' => t('Library patron cart'),
  );
}

/**
 * Callback function to supply a list of content types.
 */
function alma_cart_cart_content_type_content_types() {
  $types = array(
    'alma_cart_details' => array(
      'title' => t('User cart details'),
      'description' => t('Displays the contents of the user’s cart.'),
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
function alma_cart_cart_content_type_render($subtype, $conf, $panel_args, $context) {
  $account = isset($context->data) ? drupal_clone($context->data) : NULL;
  $block = new stdClass();

  switch ($subtype) {
    case 'alma_cart_details':
      $account = isset($context->data) ? drupal_clone($context->data) : NULL;
      if ($account->uid) {
        module_load_include('forms.inc', 'alma_cart');
        $block->content = drupal_get_form('alma_cart_forms_cart_details', $account);
      }
      break;
  }

  return $block;
}

/**
 * Return the pane titles.
 */
function alma_cart_cart_content_type_admin_title($subtype, $conf, $context) {
  switch($subtype) {
    case 'alma_cart_details':
      return t('“@s”: Cart details', array('@s' => $context->identifier));
  }
}

/**
 * Returns an edit form for the content type.
 *
 * We're not actually doing anything here, but we need to have this hook 
 * for this content type to work.
 */
function alma_cart_cart_content_type_edit_form(&$form, &$form_state) {
  return $form;
}

