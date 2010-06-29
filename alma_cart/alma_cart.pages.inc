<?php
// $Id$

/**
 * Page callback to handle adding stuff to the cart.
 *
 * Handles both AJAX and regular requests, presenting the user with the 
 * opportunity to log in if needed. 
 */
function alma_cart_page_add_to_cart($item_id) {
  module_load_include('client.inc', 'ting');
  $is_ahah = alma_cart_detect_ahah();
  $object = ting_get_object_by_local_id($item_id, TRUE);
  $account = $GLOBALS['user'];

  // Get creds, redirect if not ahah.
  $creds = alma_user_get_credentials($account, !$is_ahah);

  // First, let's check if this is in the cart already :)
  $cart_item_exists = (boolean) db_result(db_query("SELECT created FROM {alma_cart} WHERE uid = %d AND alma_id = '%s';", $account->uid, $object->localId));
  $reservation_exists = alma_client_user_has_reservation($creds['borr_card'], $creds['pin_code'], $reservable);

  // If client wants JSON (usually AJAX requests), let's give it to them.
  if ($is_ahah) {
    $reply = array();
    if (!$account->uid || !$creds) {
      $reply['status'] = 'error';
      $reply['error_type'] = 'not-logged-in';
      $reply['title'] = t('Not logged in');
      $reply['message'] = t('You have to log in to put books in your cart');
    }
    elseif ($cart_item_exists) {
      $reply['status'] = 'error';
      $reply['error_type'] = 'already-in-cart';
      $reply['title'] = t('Already in cart');
      $reply['message'] = t('@item is already in your cart.', array('@item' => $object->title));
    }
    elseif ($reservation_exists) {
      $reply['status'] = 'error';
      $reply['error_type'] = 'reservation-exists';
      $reply['title'] = t('Reservation exists');
      $reply['message'] = t('@item is already reserved.', array('@item' => $object->title));
    }
    else {
      if (alma_cart_add_ting_object($object) == SAVED_NEW) {
        $reply['status'] = 'success';
        $reply['title'] = t('Success');
        $reply['message'] = t('@item is now in your cart.', array('@item' => $object->title));
        $reply['cart_link'] = url('user/' . $account->uid . '/cart');
      }
      else {
        $reply['status'] = 'error';
        $reply['error_type'] = 'cart-save-error';
        $reply['title'] = t('Cart saving error');
        $reply['message'] = t('An error occurred while saving to the cart. Please try again.');
      }
    }

    // We are returning JavaScript, so tell the browser.
    drupal_set_header('Content-Type: application/json; charset=utf-8');
    echo json_encode($reply);
    return;
  }

  // For regular requests, we present forms, etc.
  if ($cart_item_exists) {
    drupal_set_message(t('@item is already in your cart.', array('@item' => $object->title)) , 'warning');
    return drupal_goto('user/' . $account->uid . '/cart');
  }

  if ($reservation_exists) {
    drupal_set_message(t('@item is already reserved.', array('@item' => $object->title)), 'warning');
    return drupal_goto('user/' . $account->uid . '/status');
  }

  // Present a confirmation form.
  return drupal_get_form('alma_cart_confirm_form', $object, 'add');
}

/**
 * Reserve an item immediately.
 */
function alma_cart_page_reserve_item($item_id, $reservable = NULL) {
  $branch_names = alma_client_get_branches(FALSE);
  if (is_null($reservable)) { $reservable = $item_id; }
  module_load_include('client.inc', 'ting');
  $is_ahah = alma_cart_detect_ahah();
  // Get object, enrich if not ahah. 
  $object = ting_get_object_by_local_id($item_id, !$is_ahah);
  if (!$object) {
    return drupal_not_found(); 
  }

  $account = $GLOBALS['user'];

  // Get creds, redirect if not ahah.
  $creds = alma_user_get_credentials($account, !$is_ahah);

  $reservation_exists = alma_client_user_has_reservation($creds['borr_card'], $creds['pin_code'], $reservable);

  // If client wants JSON (usually AJAX requests), let's give it to them.
  if ($is_ahah) {
    $reply = array();
    if (!$account->uid || !$creds) {
      $reply['status'] = 'error';
      $reply['error_type'] = 'not-logged-in';
      $reply['title'] = t('Not logged in');
      $reply['message'] = t('You have to log in to make reservations.');
    }
    elseif (!isset($account->preferred_branch) || !isset($branch_names[$account->preferred_branch])) {
      $reply['status'] = 'error';
      $reply['error_type'] = 'no-preferred-branch';
      $reply['title'] = t('Preferred branch not set');
      $reply['message'] = t('To make quick reservations, you need to have a preferred branch set. You can set one up at your !profile_settings.', 
        array('!profile_settings' => l(t('profile settings page'), 'user/' . $account->uid . '/profile')
      ));

    }
    elseif ($reservation_exists) {
      $reply['status'] = 'error';
      $reply['error_type'] = 'reservation-exists';
      $reply['title'] = t('Reservation exists');
      $reply['message'] = t('@item is already reserved.', array('@item' => $object->title));
    }
    else {
      $reservation = alma_client_add_reservation($creds['borr_card'], $creds['pin_code'], array(
        'id' => $reservable,
        'pickup_branch' => $account->preferred_branch,
      ));

      if ($reservation) {
        $reply['status'] = 'success';
        $reply['title'] = t('Success');
        $reply['message'] = t('You have reserved @item and will be notified when its ready for pickup at @branch.', array(
          '@item' => $object->title,
          '@branch' => $branch_names[$account->preferred_branch],
        ));
        $reply['cart_link'] = url('user/' . $account->uid . '/status');
      }
      else {
        $reply['status'] = 'error';
        $reply['error_type'] = 'reservation-error';
        $reply['title'] = t('Reservation error');
        $reply['message'] = t('An error occurred while reserving item. Please try again.');
      }
    }

    // We are returning JavaScript, so tell the browser.
    drupal_set_header('Content-Type: application/json; charset=utf-8');
    echo json_encode($reply);
    return;
  }

  // For regular requests.
  $creds = alma_user_get_credentials($account);
  if ($reservation_exists) {
    drupal_set_message(t('@item is already reserved.', array('@item' => $object->title)), 'warning');
    return drupal_goto('user/' . $account->uid . '/status');
  }

  return drupal_get_form('alma_cart_confirm_form', $account, $object, 'reserve');
}

/**
 * Alma cart confirmation form.
 *
 * @param array $form_state
 *    Drupal form API state.
 * @param stdClass $account
 *    Drupal user account. 
 * @param TingClientObject $object
 *    The object we're acting on.
 * @param string $action
 *    The type of action we're taking.
 */
function alma_cart_confirm_form(&$form_state, $account, $object, $action) {
  // Question keyed by action.
  $questions = array(
    'add' => t('Add %book to your cart?', array('%book' => $object->title)),
    'reserve' => t('Reserve %book immediately?', array('%book' => $object->title)),
  );

  // Destination URL keyed by action.
  $urls = array(
    'add' => 'user/' . $account->uid . '/cart',
    'reserve' => 'user/' . $account->uid . '/status',
  );

  // Send some form values along for the submit step.
  $form = array();

  $form['account'] = array(
    '#type' => 'value',
    '#value' => $account,
  );

  $form['object'] = array(
    '#type' => 'value',
    '#value' => $object,
  );

  $form['action'] = array(
    '#type' => 'value',
    '#value' => $action,
  );

  $form['dest_url'] = array(
    '#type' => 'value',
    '#value' => $urls[$action],
  );

  return confirm_form($form, $questions[$action], $urls[$action], '');
}

/**
 * Submit handler for Alma cart confirmation form.
 */
function alma_cart_confirm_form_submit($form, &$form_state) {
  switch ($form_state['values']['action']) {
    case 'add':
      // Save the item to the cart table.
      alma_cart_add_ting_object($form_state['values']['object']);
      break;
    case 'reserve':
      $creds = alma_user_get_credentials($form_state['values']['account'], FALSE);
      if ($creds) {
        $reservation = alma_client_add_reservation($creds['borr_card'], $creds['pin_code'], array(
          'id' => $form_state['values']['object']->localId,
          'pickup_branch' => $form_state['values']['account']->preferred_branch,
        ));
        if ($reservation) {
          drupal_set_message(t('@item is now on your reservation list.', array('@item' => $form_state['values']['object']->title)));
        }
        else {
          drupal_set_message(t('Reservation of @item failed.', array('@item' => $form_state['values']['object']->title)));
        }
      }
      break;
  }

  $form_state['redirect'] = $form_state['values']['dest_url'];
}

