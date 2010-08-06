// $Id$
/**
 * @file alma.availability.js
 * JavaScript behaviours for fetching and displaying availability.
 */

// Container object for all our availability stuff.
Drupal.almaAvailability = {
  id_matcher: /ting-item-(\d+)/,
  id_list: []
};

/**
 * Helper function to find and store all ting item ids.
 */
Drupal.almaAvailability.find_ids = function () {
  $("div.ting-item").each(function () {
    var match = Drupal.almaAvailability.id_matcher.exec(this.id);

    if (match) {
      Drupal.almaAvailability.id_list.push(match[1]);
    }
  });
};

/**
 * Get details for all ting items found.
 */
Drupal.almaAvailability.get_details = function (callback) {
  // If the id_list is empty, try and find ids again.
  if (Drupal.almaAvailability.id_list.length === 0) {
    Drupal.almaAvailability.find_ids();
  }

  if (Drupal.almaAvailability.id_list.length > 0) {
    $.getJSON(Drupal.settings.basePath + 'alma/item/' + Drupal.almaAvailability.id_list.join(',') + '/details', {}, callback);
  }
};

/**
 * Get availability for all ting items found.
 *
 * This call is more light-weight than get_details, and thus more
 * suitable if you have multiple ting items on a page.
 */
Drupal.almaAvailability.get_availability = function (callback) {
  // If the id_list is empty, try and find ids again.
  if (Drupal.almaAvailability.id_list.length === 0) {
    Drupal.almaAvailability.find_ids();
  }

  if (Drupal.almaAvailability.id_list.length > 0) {
    $.getJSON(Drupal.settings.basePath + 'alma/item/' + Drupal.almaAvailability.id_list.join(',') + '/availability', {}, callback);
  }
};

/**
 * Availability information for all pages.
 *
 * Try to find Ting items and stuff availability data into them.
 */
Drupal.almaAvailability.updateStatus = function (data, textStatus) {
  $.each(data, function(itemId, itemData) {
    var $item = $('#ting-item-' + itemId);
    if (!itemData.show_reservation_button) {
      $item.find('.alma-status')
        .addClass('unreservable')
        .removeClass('waiting')
        .text(Drupal.t('not reservable'))
      .end()
      .find('ul.alma-cart-buttons > li > a')
        .addClass('disabled');
    }
    else if (itemData.available_count > 0) {
      $item.find('.alma-status')
        .addClass('available')
        .removeClass('waiting')
        .text(Drupal.t('available'));
    }
    else if (itemData.reservation === 0) {
      $item.find('.alma-status')
        .addClass('unavailable')
        .removeClass('waiting')
        .text(Drupal.t('unavailable'));
    }
    else {
      $item.find('.alma-status')
        .addClass('unavailable')
        .addClass('reserved')
        .removeClass('waiting')
        .text(Drupal.t('reserved'));
    }
  });
};

/**
 * Format a holding as readable text.
 */
Drupal.almaAvailability.formatHolding = function (item, holding) {
  var locations = [Drupal.settings.alma.branches[holding.branch_id]];

  // Take each location type ID and look it up in our department data.
  // If a match is found, add it to the locations array.
  $.each(['department_id', 'location_id', 'sublocation_id', 'collection_id'], function (index, key) {
    if (holding[key] && Drupal.settings.alma.departments[holding[key]]) {
      locations.push(Drupal.settings.alma.departments[holding[key]]);
    }
  });

  if (holding.shelf_mark) {
    locations.push(holding.shelf_mark);
  }

  return locations.join(' → ');
};

