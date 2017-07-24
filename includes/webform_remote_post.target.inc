<?php

/**
 * @file
 * Webform Remote Post URL single target edit page.
 */

/**
 * Form for configuring an individual remote post target settings.
 * @param array $form
 * @param array $form_state
 * @param array $webform_node An array representing a webform node.
 * @param integer $target A webform remote post target.
 */
function webform_remote_post_target_edit_form($form, &$form_state, $webform_node, $target = array()){

  #drupal_set_message('In ' . __FILE__ . ' line ' . __LINE__ . ', form is: ' . print_r($form, true));
  $form['#tree'] = TRUE;
  $form['#node'] = $webform_node;

  #drupal_set_message(print_r($target, true));
  // Internal to the form and never displayed to the screen.
  $form['nid'] = array(
    '#type'  => 'value',
    '#value' => $webform_node->nid,
  );

  if (isset($target['tid'])) {
    $form['tid'] = array(
      '#type'  => 'value',
      '#value' => $target['tid'],
    );
  }

  // Form elements for a remote post target settings. Actually rendered by
  // the theme_webform_remote_post_target_edit_form function below.
  $form['edit']['label'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Label'),
    '#description'   => t('The name of the remote system as displayed in the table above. E.g. "My CRM system"'),
    '#size'          => 20,
    '#maxlength'     => 60,
    '#required'      => FALSE,
    '#default_value' => $target['label'],
  );

  $form['edit']['url'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Target URL'),
    '#description'   => t('The full URL to POST to. E.g. http://www.mycrm.com/form_handler.php'),
    '#size'          => 100,
    '#maxlength'     => 500,
    '#required'      => TRUE,
    '#default_value' => $target['url'],
  );

  $form['edit']['type'] = array(
    '#type'          => 'select',
    '#title'         => t('Post Type'),
    '#default_value' => (isset($target['type'])) ? $target['type'] : 'x-www-form-urlencoded',
    '#description'   => t('Use x-www-form-urlencoded if unsure, as it is the default format for HTML forms. You also have the option to post data in <a href="http://www.json.org/" target="_blank">JSON</a> format.'),
    '#required'      => TRUE,
    '#options'       => array(
      'x-www-form-urlencoded' => t('x-www-form-urlencoded'),
      'json'                  => t('JSON'),
    ),
  );

  $form['edit']['mode'] = array(
    '#type'          => 'radios',
    '#title'         => t('Mode'),
    '#description'   => t('Specify whether this target should be posted to before or after saving the submission data to the database.'),
    '#default_value' => isset($target['mode']) ? $target['mode'] : WEBFORM_REMOTE_POST_MODE_POST,
    '#required'      => TRUE,
    '#options'       => array(
      WEBFORM_REMOTE_POST_MODE_PRE  => t('Pre-save'),
      WEBFORM_REMOTE_POST_MODE_POST => t('Post-save'),
    ),
  );

  $form['edit']['referer'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Referer'),
    '#description'   => t('Value for the Referer header. $GLOBALS[\'base_url\'] will be used if this is left blank.'),
    '#size'          => 100,
    '#maxlength'     => 500,
  );

  $form['edit']['extra'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Extra'),
    '#description'   =>
      t('Provide extra data to include in post here. One item per line, in the format: key|data') .
      '<br /><small><strong>' .
      t('Note:') . '</strong>' .
      t('If you specify an existing field name (key) here it will OVERRIDE the submitted value,
        providing just that key without an associated value, will OVERRIDE that submitted field with a NULL value.') . '</small>',
    '#default_value' => (isset($target['extra']) && !is_null($target['extra']))
      ? unserialize($target['extra'])
      : '',
  );

  // Add the submit button.
  $form['submit'] = array(
    '#type'   => 'submit',
    '#value'  => t('Save remote post settings'),
    '#weight' => 20,
  );
  #drupal_set_message('In ' . __FILE__ . ' line ' . __LINE__ . ', form is: ' . print_r($form, true));
  return $form;
}

/**
 * Theme the remote post settings section of the node form.
 * @param array $variables
 *   An array representing the form, with these keys:
 *   - form: the form array.
 *
 * @return array
 *   Formatted HTML form, ready for display.
 */
function theme_webform_remote_post_target_edit_form($variables) {
  $form = $variables['form'];
  $children = element_children($form, TRUE);
  return drupal_render_children($form, $children);
}

/**
 * Submit handler for webform_remote_post_target_edit_form().
*/
function webform_remote_post_target_edit_form_submit($form, &$form_state) {
  $target = array();
  #drupal_set_message('In ' . __FILE__ . ' line ' . __LINE__ . ', $form_state is: ' . print_r($form_state, true));

  // Optional fields:
  if (!empty($form_state['values']['edit']['label'])) {
    $target['label'] = trim($form_state['values']['edit']['label']);
  }

  if (!empty($form_state['values']['edit']['extra'])) {
    $target['extra'] = serialize($form_state['values']['edit']['extra']);
  }
  
  if (!empty($form_state['values']['edit']['referer'])) {
    $target['referer'] = trim($form_state['values']['edit']['referer']);
  }

  // Required fields:
  $target['mode'] = $form_state['values']['edit']['mode'];
  $target['type'] = $form_state['values']['edit']['type'];
  $target['nid']  = $form_state['values']['nid'];
  $target['url']  = trim($form_state['values']['edit']['url']);


  if (!isset($form_state['values']['tid'])) {
    // TODO: Like in webform, this is not race-condition safe.
    // Switch to using transactions as well?
    $next_id_query = db_select('webform_remote_post_targets')->condition('nid', $target['nid']);
    $next_id_query->addExpression('MAX(tid) + 1', 'tid');
    $next_id = $next_id_query->execute()->fetchField();
    if ($next_id == NULL) {
      $target['tid'] = 1;
    } else {
      $target['tid'] = $next_id;
    }
    $success = drupal_write_record('webform_remote_post_targets', $target);
  } else {
    $target['tid'] = trim($form_state['values']['tid']);
    #drupal_set_message('In ' . __FILE__ . ' line ' . __LINE__ . ', $target is: ' . print_r($target, true));
    $success = drupal_write_record('webform_remote_post_targets', $target, array('nid', 'tid'));
  }

  #drupal_set_message('In ' . __FILE__ . ' line ' . __LINE__ . ', $success is: ' . print_r($success, true));

  // Write a log if we weren't able to save the target successfully.
  if ($success === FALSE) {
    watchdog(
      'webform_remote_post',
      'There was an error writing a new remote post target to the database.',
      array(),
      WATCHDOG_ERROR
    );
  }

  $form_state['redirect'] = 'node/' . $form['#node']->nid . '/webform/targets/';
}
