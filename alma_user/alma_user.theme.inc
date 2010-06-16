<?php
// $Id$

/**
 * Theming of a records list.
 *
 * Usually a list of reservations, books, etc, coming from Alma.
 */
function theme_alma_user_record_list($records) {
  module_load_include('client.inc', 'ting');
  $items = array();

  foreach ($records as $rec) {
    $items[] = theme('alma_user_list_item', $rec);
  }

  return theme('item_list', $items, NULL, 'ul', array('class' => 'record-list'));
}

/**
 * Theming of reservation details form.
 */
function theme_alma_user_forms_reservation_details($form) {
  $date_format = variable_get('date_format_date_short', 'Y-m-d');
  $output = '';
  $branches = alma_client_get_branches();
  module_load_include('client.inc', 'ting');

  if (!empty($form['reservation_data']['#grouped']['fetchable'])) {
    $header = array(
      t('Select'),
      '',
      t('Title'),
      t('Pickup number'),
      t('Pickup by'),
      t('Pickup branch'),
    );

        $colgroups = array(
      array(
        array(
          'class' => 'col-selection',
        ),
      ),
      array(
        array(
          'class' => 'col-image',
        ),
      ),
      array(
        array(
          'class' => 'col-title',
        ),
      ),
      array(
        array(
          'class' => 'col-pickup-number',
        ),
      ),
      array(
        array(
          'class' => 'col-pickup-by',
        ),
      ),
      array(
        array(
          'class' => 'col-pickup-branch',
        ),
      ),
    );
    
    $rows = array();

    foreach ($form['reservation_data']['#grouped']['fetchable'] as $item) {
      $cells = array();

      if (isset($form['reservations'][$item['id']])) {
        $cells['checkbox'] = array(
          'class' => 'checkbox',
          'data' => drupal_render($form['reservations'][$item['id']]),
        );
      }
      else {
        $cells['checkbox'] = array(
          'class' => 'checkbox empty',
          'data' => '–',
        );
      }

      $image_url = ting_covers_faust_url($item['record_id'], '80_x');
      $cells['image'] = array(
        'class' => 'image',
        'data' => ($image_url) ? theme('image', $image_url, '', '', NULL, FALSE) : '',
      );

      $cells['title'] = array(
        'class' => 'title',
        'data' => theme('alma_user_list_item', $item) . ' (<span class="reservation-number">' . t('Res. no @num', array('@num' => $item['id'])) . '</span>)',
      );

      $cells['pickup_number'] = array(
        'class' => 'pickup_number',
        'data' => $item['pickup_number'],
      );

      $cells['pickup_expire_date'] = array(
        'class' => 'pickup_expire_date',
        'data' => alma_format_date($item['pickup_expire_date'], $date_format),
      );

      $cells['pickup_branch'] = array(
        'class' => 'pickup_branch',
        'data' => $branches[$item['pickup_branch']],
      );

      $rows[] = $cells;
    }

    $output .= theme('table', $header, $rows, array('id' => 'reservations-fetchable', 'colgroups' => $colgroups), t('Reservations ready for pickup'));
  }

  if (!empty($form['reservation_data']['#grouped']['active'])) {
    $header = array(
      t('Select'),
      '',
      t('Title'),
      t('Reserved'),
      t('Valid to'),
      t('Pickup branch'),
      t('Queue number'),
    );
    
    $colgroups = array(
      array(
        array(
          'class' => 'col-selection', 
        ),
      ),
      array(
        array(
          'class' => 'col-image',
        ),
      ),
      array(
        array(
          'class' => 'col-title-res',
        ),
      ),
      array(
        array(
          'class' => 'col-reservation', 
        ),
      ),
      array(
        array(
          'class' => 'col-valied-to', 
        ),
      ),
      array(
        array(
          'class' => 'col-pickup-branch', 
        ),
      ),
      array(
        array(
          'class' => 'col-queue-number', 
        ),
      ),
    );

    $rows = array();
    foreach ($form['reservation_data']['#grouped']['active'] as $item) {
      $cells = array();

      if (isset($form['reservations'][$item['id']])) {
        $cells['checkbox'] = array(
          'class' => 'checkbox',
          'data' => drupal_render($form['reservations'][$item['id']]),
        );
      }
      else {
        $cells['checkbox'] = array(
          'class' => 'checkbox empty',
          'data' => '–',
        );
      }

      $image_url = ting_covers_faust_url($item['record_id'], '80_x');
      $cells['image'] = array(
        'class' => 'image',
        'data' => ($image_url) ? theme('image', $image_url, '', '', NULL, FALSE) : '',
      );

      $cells['title'] = array(
        'class' => 'title',
        'data' => theme('alma_user_list_item', $item) . ' (<span class="reservation-number">' . t('Res. no @num', array('@num' => $item['id'])) . '</span>)',
      );

      $cells['create_date'] = array(
        'class' => 'create_date',
        'data' => alma_format_date($item['create_date'], $date_format),
      );

      $cells['valid_to'] = array(
        'class' => 'valid_to',
        'data' => alma_format_date($item['valid_to'], $date_format),
      );

      $cells['pickup_branch'] = array(
        'class' => 'pickup_branch',
        'data' => $branches[$item['pickup_branch']],
      );

      $cells['queue_no'] = array(
        'class' => 'queue_no',
        'data' => $item['queue_no'],
      );

      $rows[] = array(
        'data' => $cells,
        'class' => 'active-reservations',
      );
    }

    $output .= theme('table', $header, $rows, array('id' => 'reservations-active', 'colgroups' => $colgroups), t('Active reservations'));
  }

  if (empty($output)) {
    return t('No reservations found.');
  }

  $output .= '<div class="update-controls clear-block">';
  $output .= drupal_render($form['new_expiry']);
  $output .= drupal_render($form['new_pickup_branch']);
  $output .= '</div>';

  // fisk
  $output .= '<div class="update-controls-button clear-block">';
  $output .= drupal_render($form['update']);
  $output .= drupal_render($form['remove']);
  $output .= '</div>';
  
  $output .= drupal_render($form);

  return $output;
}

/**
 * Theming of loan details form.
 */
function theme_alma_user_forms_loan_details($form) {
  $date_format = variable_get('date_format_date_short', 'Y-m-d');
  module_load_include('client.inc', 'ting');
  $groups = array();
  $output = '';
  $header = array(t('Select'), '', t('Title'), t('Loan date'), t('Due date'));

  $colgroups = array(
    array(
      array(
        'class' => 'col-selection',
      ),
    ),
    array(
      array(
        'class' => 'col-image',
      ),
    ),
    array(
      array(
        'class' => 'col-title',
      ),
    ),
    array(
      array(
        'class' => 'col-loan-date',
      ),
    ),
    array(
      array(
        'class' => 'col-due-date',
      ),
    ),
  );

  foreach ($form['loan_data']['#grouped'] as $date => $group) {
    // Overdue loans get preferential treatment. No checkboxes here.
    if ($date == 'overdue') {
      $table_title = t('Overdue loans');
    }
    // The normal loans get grouped by due date.
    else {
      if ($date == 'due') {
        $table_title = t('Due today');
      }
      else {
        $table_title = t('Due in @count days, @date', array('@date' => date('d/m/y', strtotime($date)), '@count' => ceil((strtotime($date) - $_SERVER['REQUEST_TIME']) / 86400)));
      }
    }

    $rows = array();
    foreach ($group as $loan_id) {
      $loan = $form['loan_data']['#value'][$loan_id];
      $cells = array();

      if (isset($form['loans'][$loan_id])) {
        $cells['checkbox'] = array(
          'class' => 'checkbox',
          'data' => drupal_render($form['loans'][$loan_id]),
        );
      }
      else {
        $cells['checkbox'] = array(
          'class' => 'checkbox empty',
          'data' => '–',
        );
      }

      $image_url = ting_covers_faust_url($loan['record_id'], '80_x');
      $cells['image'] = array(
        'class' => 'image',
        'data' => ($image_url) ? theme('image', $image_url, '', '', NULL, FALSE) : '',
      );

      $cells['title'] = array(
        'class' => 'title',
        'data' => theme('alma_user_list_item', $loan) . ' (<span class="material-number">' . t('Mat. no @num', array('@num' => $loan['id'])) . '</span>)',
      );

      $cells['loan_date'] = array(
        'class' => 'loan_date',
        'data' => alma_format_date($loan['loan_date'], $date_format),
      );

      $cells['due_date'] = array(
        'class' => 'due_date',
        'data' => alma_format_date($loan['due_date'], $date_format),
      );

      $rows[] = array(
        'data' => $cells,
        'class' => ($checkbox) ? 'selectable' : 'immutable',
      );
    }

    if (!empty($rows)) {
      $output .= theme('table', $header, $rows, array('colgroups' => $colgroups), $table_title);
    }
  }
  if (empty($output)) {
    return t('No loans found.');
  }

  $output .= drupal_render($form);
  return $output;
}

/**
 * Theming of debt details form.
 */
function theme_alma_user_forms_debts_details($form) {
  $date_format = variable_get('date_format_date_short', 'Y-m-d');
  $header = array(
    t('Select'),
    t('Note'),
    t('Date'),
    t('Amount'),
  );

  $colgroups = array(
    array(
      array(
        'class' => 'col-selection',
      ),
      array(
        'class' => 'col-note',
      ),
      array(
        'class' => 'col-date',
      ),
      array(
        'class' => 'col-amount',
      ),
    ),
  );

  $rows = array();
  foreach ($form['debt_data']['#value']['debts'] as $key => $data) {
    $rows[$data['id']] = array(
      drupal_render($form['debts'][$data['id']]),
      array (
        'class' => 'note',
        'data' => $data['note']
      ),
      array (
        'class' => 'date',
        'data' => alma_format_date($data['date'], $date_format),
      ),
      array (
        'class' => 'amount',
        'data' => $data['amount_formatted'],
      ),
    );
  }

  $output = theme('table', $header, $rows, array('colgroups' => $colgroups));
  $output .= drupal_render($form);
  return $output;
}

/**
 * Render label for reservation or loan in a list.
 */
function theme_alma_user_list_item($item) {
  // First try looking up the title and book data from Ting.
  if (intval($item['id']) > 0) {
    $ting_object = ting_get_object_by_local_id($item['record_id']);
    if ($ting_object) {
      return theme('ting_object_title', $ting_object);
    }
  }

  // If Ting data is not available, get it from Alma instead.
  $data = alma_client_catalogue_record_detail($item['record_id']);
  $details = $data['records'][$item['record_id']];

  $output = '<span class="title">' . $details['titles'][0] . '</span> ';

  if (!empty($details['authors'])) {
    $output .= '<span class="byline">' . t('by') . '</span> ';
    $output .= implode(', ', $details['authors']);
  }

  // Remote loans have negative IDs (undocumented Axiell "feature").
  if (intval($item['id']) < 0) {
    $output .= ' <span class="remote">(' . t('remote loan') . ')</span>';
  }

  return $output;
}

