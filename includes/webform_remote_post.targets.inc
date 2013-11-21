<?php

/**
 * @file
 * Webform Remote Post URL targets administration pages.
 */

/**
 * List remote post targets associated with a given webform.
 */
function webform_remote_post_targets_form($form_state, $node) {
  // Adds the node object to the $form array.
  $form['#node'] = $node;

  // Adds the webform Node ID to the form.
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  // Retrieves the associated remote post targets from the database.
  $query = db_query('SELECT * FROM {webform_remote_post_targets} WHERE nid = %d', $node->nid);

  // Constructs an array of associated remote post targets indexed by Target ID.
  $targets = array();
  while ($target = db_fetch_array($query)) {
    $targets[$target['tid']] = $target;
  }

  // Constructs a form from the array of associated remote post targets.
  foreach ($targets as $tid => $data) {
    $status = ($data['enabled'] ? t('Enabled') : t('Disabled'));
    $form['targets'][$tid]['status'] = array(
      '#value' => check_plain($status),
    );
    $form['targets'][$tid]['label'] = array(
      '#value' => check_plain($data['label']),
    );
    $form['targets'][$tid]['url'] = array(
      '#value' => check_plain($data['url']),
    );
    $form['targets'][$tid]['type'] = array(
      '#value' => check_plain($data['type']),
    );
  }

  // Constructs a form for adding a new remote post target.
  $form['add']['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#description' => t('The name of the remote system as displayed in the table above. E.g. "My CRM system"'),
    '#size' => 20,
    '#maxlength' => 60,
    '#required' => TRUE,
  );
  $form['add']['url'] = array(
    '#type' => 'textfield',
    '#title' => t('Target URL'),
    '#description' => t('The full URL to POST to. E.g. http://www.mycrm.com/form_handler.php'),
    '#size' => 100,
    '#maxlength' => 500,
    '#required' => TRUE,
  );
  $form['add']['type'] = array(
    '#type' => 'select',
    '#title' => t('Post Type'),
    '#options' => array(
      'x-www-form-urlencoded' => t('x-www-form-urlencoded'),
      'json' => t('JSON'),
    ),
    '#default_value' => 'x-www-form-urlencoded',
    '#description' => t('Use x-www-form-urlencoded if unsure, as it is the default format for HTML forms. You also have the option to post data in <a href="@url" target="_blank">JSON</a> format.', array('@url' => url('http://www.json.org/', array('external' => TRUE)))),
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
    '#weight' => 45,
  );

  return $form;
}

/**
 * Theme the node components form. Use a table to organize the components.
 *
 * @param array $form
 *   An array representing the form.
 *
 * @return array
 *   Formatted HTML form, ready for display.
 */
function theme_webform_remote_post_targets_form($form) {
  // Constructs an array of table header values.
  $header = array(
    t('Status'),
    t('Label'),
    t('Target URL'),
    t('Post Type'),
    t('Operations'),
  );

  // Constructs an array of table row values.
  $rows = array();
  if (!empty($form['targets'])) {
    foreach (element_children($form['targets']) as $tid) {
      // Adds each remote post target to a table row.
      $update = ($form['targets'][$tid]['status']['#value'] == 'Enabled' ? t('Disable') : t('Enable'));
      $rows[] = array(
        drupal_render($form['targets'][$tid]['status']),
        drupal_render($form['targets'][$tid]['label']),
        drupal_render($form['targets'][$tid]['url']),
        drupal_render($form['targets'][$tid]['type']),
        l($update, 'node/' . $form['#node']->nid . '/webform/targets/' . $tid . '/' . strtolower($update)),
        l(t('Delete'), 'node/' . $form['#node']->nid . '/webform/targets/' . $tid . '/delete'),
      );
    }
  }
  else {
    $rows[] = array(array('data' => t('No remote target URLs specified yet. Add a target URL below.'), 'colspan' => 6));
  }

  // Adds a row containing form elements for a new remote post target.
  $row_data = array(
    array('colspan' => 5, 'data' => drupal_render($form['add'])),
    array('colspan' => 1, 'data' => drupal_render($form['add_button'])),
  );
  $rows[] = array('data' => $row_data, 'class' => array('webform-remote-post-add-target-form'));

  $output = theme('table', $header, $rows);

  foreach (element_children($form) as $key) {
    if (!empty($form[$key])) {
      $output .= drupal_render($form[$key]);
    }
  }

  return $output;
}

/**
 * Validation handler for webform_remote_post_targets_form().
 */
function webform_remote_post_targets_form_validate($form, &$form_state) {
  if (!valid_url(trim($form_state['values']['url']))) {
    $url = trim($form_state['values']['url']);
    form_set_error('url', t('The supplied URL "@url" does not appear valid.', array('@url' => $url)));
  }
}

/**
 * Submission handler for webform_remote_post_forwards_form().
 */
function webform_remote_post_targets_form_submit($form, &$form_state) {
  // Constructs the remote post target object.
  $target = new stdClass();
  $target->nid = trim($form_state['values']['nid']);
  $target->url = trim($form_state['values']['url']);
  $target->label = trim($form_state['values']['label']);
  $target->type = trim($form_state['values']['type']);

  // Writes the remote post target object to the database.
  $success = drupal_write_record('webform_remote_post_targets', $target);

  // Logs unsuccessful updates.
  if ($success === FALSE) {
    watchdog('webform_remote_post', 'There was an error writing a new remote post target to the database.', array(), WATCHDOG_ERROR);
    drupal_set_message(t('The remote post target could not be created.'), 'error');
  }
  else {
    drupal_set_message(t('The remote post target was successfully created.'));
  }
}

/**
 * Page callback for enabling and disabling remote post targets.
 *
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @param object $node
 *   The node object passed by webform_menu_load().
 *
 * @param int $tid
 *   The Target ID passed into the URL.
 *
 * @param string $flag
 *   The requested state passed into the URL.
 */
function webform_remote_post_targets_update($form_state, $node, $tid, $flag) {
  // Converts $flag to an integer representing the requested state.
  $flag = ($flag == 'enable' ? 1 : 0);

  // Constructs the remote post target object.
  $target = new stdClass();
  $target->nid = $node->nid;
  $target->tid = $tid;
  $target->enabled = $flag;

  // Updates the remote post target's database record.
  $success = drupal_write_record('webform_remote_post_targets', $target, array('nid', 'tid'));

  // Logs unsuccessful updates.
  if ($success === FALSE) {
    watchdog('webform_remote_post', 'There was an error updating a remote post target.', array(), WATCHDOG_ERROR);
    drupal_set_message(t('The remote post target could not be updated.'), 'error');
  }
  else {
    drupal_set_message(t('The remote post target was successfully updated.'));
    drupal_goto('node/' . $node->nid . '/webform/targets');
  }
}

/**
 * Page callback for deleting remote post targets.
 *
 * @param array $form_state
 *   A keyed array containing the current state of the form.
 *
 * @param object $node
 *   The node object passed by webform_menu_load().
 *
 * @param int $tid
 *   The Target ID passed into the URL.
 */
function webform_remote_post_targets_delete($form_state, $node, $tid) {
  // Deletes the remote post target's database record.
  $success = db_query('DELETE FROM {webform_remote_post_targets} WHERE nid = %d AND tid = %d', $node->nid, $tid);

  // Logs unsuccessful updates.
  if ($success === FALSE) {
    watchdog('webform_remote_post', 'There was an error deleting a remote post target.', array(), WATCHDOG_ERROR);
    drupal_set_message(t('The remote post target could not be deleted.'), 'error');
  }
  else {
    drupal_set_message(t('The remote post target was successfully deleted.'));
    drupal_goto('node/' . $node->nid . '/webform/targets');
  }
}
