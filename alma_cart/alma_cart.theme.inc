<?php
// $Id$

/**
 * Rendering of Alma cart item.
 */
function theme_alma_cart_item($item) {
  $output = '<span class="title">' . l($item['title'], 'ting/object/' . $item['ting_id'])  . '</span> ';
  $output .= t('by');
  $output .= ' <span class="author">' . $item['author'] . '</span>';
  $output .= ' <span class="type">' . $item['type'] . '</span>';
  return $output;
}

/**
 * Theme function to render reservation buttons for a TingClientObject.
 *
 * @param TingClientObject $object
 * @return string
 */
function theme_alma_cart_reservation_buttons($object) {
  // Leave out reservation buttons for some record types.
  if ($object->type == 'Tidsskrift') {
    return;
  }

  jquery_ui_add('ui.dialog');
  jquery_ui_theme_load();
  drupal_add_js(drupal_get_path('module', 'alma_cart') . '/alma_cart.buttons.js');


  // Netdocuments have special proxy URLs we link to.
  if ($object->type == 'Netdokument') {
    return theme('item_list', array(
      l(t('View on-line'),  $object->online_url, array('attributes' => array('class' => 'view-online'))),
    ), NULL, 'ul', array('class' => 'buttons orange-button'));
  }
  // Audiobooks or similar have a link in their metadata we display.
  elseif ($object->type == 'Lydbog (online)') {
    if (!empty($object->record['dc:identifier']['dcterms:URI'])) {
      return theme('item_list', array(
        l(t('View on-line'),  $object->record['dc:identifier']['dcterms:URI'][0], array('attributes' => array('class' => 'view-online'))),
      ), NULL, 'ul', array('class' => 'buttons orange-button'));
    }
  }
  // For all the other object types we don’t want to provide reservation
  // buttons for.
  elseif ($object->type == 'Artikel') {
    return;
  }
  elseif (!empty($object->localId)) {
    // TODO: Check if item is reserved/in basket already.
    $output = theme('item_list', array(
      l(t('Reserve now'), 'alma/cart/reserve/' . $object->localId, array('attributes' => array('class' => 'reserve-now'))),
      l(t('Add to cart'), 'alma/cart/add/'  . $object->localId, array('attributes' => array('class' => 'add-to-cart'))),
    ), NULL, 'ul', array('class' => 'buttons alma-cart-buttons'));
    return $output;
  }
}

