<?php
/**
 * @file clients.connection.admin.inc
 * Page callbacks relating to client connection admin.
 */

/**
 * (Tentative) Interface for clients connection admin UI.
 */
interface ClientsConnectionAdminUIInterface {

  /**
   * Add configuration form elements specific to a connection's edit form.
   *
   * @param $form
   *  The form built so far. This contains the elements for the name and
   *  endpoint, but not the save button.
   *
   * @param $form_state
   *  The form state from the main form, which you probably don't need anyway.
   *
   * @return
   *  A FormAPI form array. This replaces the original form.
   *
   * @see clients_connection_form()
   * @see clients_connection_form()
   * @see clients_connection_form_submit()
   */
  function connectionSettingsForm($form, &$form_state);

}

/**
 * Form builder for editing a connection.
 *
 * Gets the class of the connection from the data in the form and calls the
 * connectionSettingsForm() method on the class to build the form.
 * This allows different elements for different connection types.
 *
 * @param $connection
 *  A connection object.
 *
 * @see clients_connection_form_submit()
 */
function clients_connection_form($form, &$form_state, $connection, $op = 'edit') {
  $form = array();

  // OBSOLETE??? TODO
  $type = $connection->type;
  $class = 'clients_connection_' . $type;


  /*
  $form['old_connection'] = array(
    '#type' => 'value',
    '#value' => $connection,
  );
  */

  $form['fake_type'] = array(
    '#type' => 'textfield',
    '#title' => t('Connection type'),
    '#description' => t('The type of this connection. May not be changed.'),
    '#value' => $connection->type,
    '#size' => 50,
    '#disabled' => TRUE,
  );

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Connection label'),
    '#default_value' => isset($connection->label) ? $connection->label : '',
    '#size' => 50,
    '#maxlength' => 100,
    '#description' => t('The human-readable name for the connection.'),
    '#required' => TRUE,
  );
  
  // Machine-readable type name.
  $form['name'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($connection->name) ? $connection->name : '',
    '#maxlength' => 32,
    '#disabled' => ($op == 'clone'),
    '#machine_name' => array(
      'exists' => 'clients_connection_load',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this connection. It must only contain lowercase letters, numbers, and underscores.'),
  );
  /*
  if ($connection->name) {
    $form['name']['#description'] .= '<br /><strong>' . t('Warning: Changing the name of an existing connection may affect any data you have stored based on that connection.') . '</strong>';
  }
  */

  $form['endpoint'] = array(
    '#type' => 'textfield',
    '#title' => t('Connection endpoint'),
    '#default_value' => isset($connection->is_new) ? '' : $connection->endpoint,
    '#size' => 100,
    '#maxlength' => 100,
    '#description' => t('Remote service URL e.g. http://mysite.com/services/endpoint'),
    '#required' => TRUE,
  );

  // Container for all form elements whose values should be serialized to the
  // configuration array.
  // Not a fieldset by default, but connection classes may choose to do this.
  $form['configuration'] = array(
    '#tree' => TRUE,
  );

  $form['#connection_type'] = $type;
  $form['#connection_class'] = $class;

  // Allow the connection class to make additions to the form. We don't use
  // the node_form() pattern because typically a connection class will want to
  // change the 'endpoint' element to add custom help text.
  $form = $connection->connectionSettingsForm($form, $form_state);

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save connection'),
  );

  return $form;
}

/**
 * Form submit handler for the connection form.
 *
 * Gets the class of the connection from the data in the form and calls the
 * connectionSettingsForm_submit() method on the class.
 * This allows different behaviour for different connection types, while
 * saving is handled here.
 *
 * @see clients_connection_form()
 */
function clients_connection_form_submit($form, &$form_state) {
  $connection = entity_ui_form_submit_build_entity($form, $form_state);
  
  // Save and go back.
  $connection->save();

  drupal_set_message(t('Connection saved.'));

  $form_state['redirect'] = 'admin/structure/clients/connections';
}

/**
 * Page callback to view a single connection.
 *
 * TODO: reimplement??
 *
 * @param $connection
 *  A loaded connection.
 */
function clients_connection_view($connection) {
  // Build summary table.
  $rows = array();
  $rows[] = array(t('Name'), check_plain($connection->name));
  $rows[] = array(t('Type'), $connection->type);
  $rows[] = array(t('Endpoint'), $connection->formatEndpoint($connection->endpoint));
  $rows[] = array(t('Storage'), $connection->export_type_label);
  foreach ($connection->configuration as $label => $val) {
    if (is_array($val)) {
      $val = implode(', ', $val); // needs to be recursive?
    }
    $rows[] = array(ucfirst(t($label)), nl2br(check_plain($val)));

  }

  return theme('table', array(
    'rows' => $rows,
  ));
}

/**
 * Page callback to test a connection.
 *
 * @param $connection
 *  A loaded connection.
 */
function clients_connection_test_form($form, &$form_state, $connection) {
  $form['#connection'] = $connection;

  $form['connection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Connection details'),
  );
  $items[] = t('Type') . ': ' . $connection->type;
  $items[] = t('Name') . ': ' . check_plain($connection->name);
  $items[] = t('Endpoint') . ': ' . $connection->endpoint;
  $form['connection']['details'] = array(
    '#value' => theme('item_list', array('items' => $items)),
  );

  // Buttons. These are themed into a vertical list with the description text alongside each one.
  $form['buttons'] = array(
    //'#theme' => 'uprpc_manual_form_button',
    '#tree' => TRUE,
  );

  // Get the core buttons provided by the connection class.
  $buttons = $connection->getTestOperations($form_state, $connection->name);

  // Allow applications that use this connection to add their own test buttons.
  drupal_alter('client_connection_test_buttons', $buttons, $form_state, $connection->name);

  // Some processing of the buttons to simplify the method of specifying them.
  // see ClientsServicesDrupal_5::getTestOperations for an example of how this works.
  foreach ($buttons as $key => $button) {
    // If the button is just a plain button, wrap it in a fieldset.
    // This allows you to return just a button if there are no extra form elements.
    if ($button['#type'] == 'submit') {
      $form['buttons'][$key] = array(
        '#type' => 'fieldset',
      );
      $form['buttons'][$key]['button'] = $button;
    }
    // Otherwise, take the whole thing: we expect it to be a fieldset.
    else {
      $form['buttons'][$key] = $button;
    }

    // In all cases, show the description for the action.
    /*
    $form['buttons'][$key]['description'] = array(
      '#value' => $form['buttons'][$key]['button']['#description'],
      '#weight' => -10,

    );
    */
    $form['buttons'][$key]['#description'] = $form['buttons'][$key]['button']['#description'];

    $form['buttons'][$key]['#tree'] = TRUE;
    $form['buttons'][$key]['button']['#key'] = $key;
  }

  // TODO: allow applications to add buttons to this connection.

  // Show the results that the button handler returned.
  foreach (array_keys($form['buttons']) as $button_id) {
    // If there is a key in the form storage with the same ID as a button,
    // the it is the result of a previous manual test.
    if (isset($form_state['storage'][$button_id])) {
      $form['buttons'][$button_id]['results'] = array(
        '#type' => 'fieldset',
        '#title' => t('Results'),
        '#collapsible' => TRUE,
      );
      $title = t($form['buttons'][$button_id]['button']['#value']);
      $data  = check_plain(print_r($form_state['storage'][$button_id], TRUE));
      $form['buttons'][$button_id]['results'][$button_id] = array(
        '#type' => 'markup',
        '#value' => "<h3>$title</h3>" . "<pre>$data</pre>",
      );
      // Clear the storage so it doesn't just accumulate.
      unset($form_state['storage'][$button_id]);
    }
  }

  // TODO: show a message if no buttons at all!

  return $form;
}

/**
 * Validate handler for the connection test page.
 */
function clients_connection_test_form_validate($form, &$form_state) {
  $connection = $form['#connection'];
  $button_key = $form_state['clicked_button']['#key'];
  // Only pass the method the values that are related to it; everything else is just cruft.
  $button_form_values = $form_state['values']['buttons'][$button_key];

  if (isset($form_state['clicked_button']['#action_validate'])) {
    $validate_handler = $form_state['clicked_button']['#action_validate'];
    $connection->$validate_handler($button_form_values);
  }
}


/**
 * Submit handler for the connection test page.
 */
function clients_connection_test_form_submit($form, &$form_state) {
  //dsm($form_state);

  $connection = $form['#connection'];

  $button_key = $form_state['clicked_button']['#key'];
  $submit_handler = $form_state['clicked_button']['#action_submit'];

  // Only pass the method the values that are related to it; everything else is just cruft.
  $button_form_values = $form_state['values']['buttons'][$button_key];

  // The submit callback is either a method on the connection object or a regular function.
  if ($form_state['clicked_button']['#action_type'] == 'method') {
    $result = $connection->$submit_handler($button_form_values);
  }
  else {
    $result = $submit_handler($button_form_values);
  }

  // Place the data returned from the connection in the form for display.
  $form_state['storage'][$button_key] = $result;
}
