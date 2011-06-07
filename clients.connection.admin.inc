<?php
/**
 * @file clients.connection.admin.inc
 * Page callbacks relating to client connection admin.
 */

/**
 * Page callback: list connections.
 */
function clients_connections_list() {
  $output = '';

  $connection_types = clients_get_connection_types();
  $connections = clients_connection_load_all();

  $rows = array();
  foreach ($connections as $name => $connection) {
    $delete_label = ($connection->export_type & EXPORT_IN_CODE) ? t('revert') : t('delete');
    $delete_link  = ($connection->export_type & EXPORT_IN_DATABASE) ? l($delete_label, "admin/build/clients/connections/$name/delete") : '';
    $type = $connection->type;
    if (isset($connection->broken_message)) {
      // Not sure where best to shove this, will take suggestions.
      $type .= ' ' . t('(broken)');
    }
    $rows[] = array(
      'name' => l($connection->name, "admin/build/clients/connections/$name/view"),
      'type' => $type,
      'endpoint' => $connection->formatEndpoint($connection->endpoint),
      'storage' => $connection->export_type_label,
      'edit' => l(t('edit'), "admin/build/clients/connections/$name/edit"),
      'test' => l(t('test'), "admin/build/clients/connections/$name/test"),
      'delete' => $delete_link,
    );
  }

  // Ensure the table has a row if there are no connections at all.
  if (!count($rows)) {
    $rows[] = array(
      array(
        'data' => t('No connections defined yet.'),
        'colspan' => '5',
      ),
    );
  }

  $headers = array(t('Name'), t('Type'), t('Endpoint'), t('Storage'), array('data' => t('Operations'), 'colspan' => 3));
  $output .= theme_table($headers, $rows);

  // Add a list of connection types that can be created.
  foreach ($connection_types as $type => $type_data) {
    //dsm($type_data);
    $items[] = l(t('Add @type connection', array('@type' => $type_data['label'])), 'admin/build/clients/connections/add/' . $type);
  }
  $output .= theme('item_list', $items);

  return $output;
}

/**
 * Menu callback for adding a new connection.
 *
 * @param $type
 *  The name of a connection type.
 */
function clients_connection_add($type) {
  // Check the type exists.
  $connection_types = clients_get_connection_types();
  if (!isset($connection_types[$type])) {
    return drupal_not_found();
  }

  drupal_set_title(t('Add @name connection', array('@name' => $type)));
  // Make a basic default connection object to pass to the form builder.
  $data = new stdClass();
  $data->type = $type;
  $data->name = '';
  $data->new = TRUE;
  // This is a bit convoluted but it's simpler to just follow the same pattern
  // that using CTools imposes on us rather than have two ways of doing it.
  $class = 'clients_connection_' . $type;
  $connection = new $class($data);
  return drupal_get_form('clients_connection_form', $connection);
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
 * @see clients_connection_form_validate()
 * @see clients_connection_form_submit()
 */
function clients_connection_form(&$form_state, $connection) {
  $form = array();

  $type = $connection->type;
  $class = 'clients_connection_' . $type;

  $form['old_connection'] = array(
    '#type' => 'value',
    '#value' => $connection,
  );

  //dsm($connection);

  $form['type'] = array(
    '#type' => 'textfield',
    '#title' => t('Connection type'),
    '#description' => t('The type of this connection. May not be changed.'),
    '#value' => $connection->type,
    '#size' => 50,
    '#disabled' => TRUE,
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Connection machine name'),
    '#default_value' => $connection->name,
    '#size' => 50,
    '#maxlength' => 100,
    '#description' => t('The connection name must contain only lowercase letters, numbers, and underscores. It must be unique.'),
    '#required' => TRUE,
  );
  if ($connection->name) {
    $form['name']['#description'] .= '<br /><strong>' . t('Warning: Changing the name of an existing connection may affect any data you have stored based on that connection.') . '</strong>';
  }

  $form['#connection_type'] = $type;
  $form['#connection_class'] = $class;

  $form += $connection->connectionSettingsForm($form_state);

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save connection'),
  );

  return $form;
}

/**
 * Form validator handler for the connection form.
 *
 * @see clients_connection_form()
 * @see clients_connection_form_submit()
 */
function clients_connection_form_validate($form, &$form_state) {
  // Validate machine name.
  if (!preg_match('!^[a-z0-9_]+$!', $form_state['values']['name'])) {
    form_set_error('name', t('The machine-readable name must contain only lowercase letters, numbers, and underscores.'));
  }

  // Machine names must be unique.
  // Work out what the machine name was before the user submitted this form.
  $old_connection = $form_state['values']['old_connection'];
  // Get all connections.
  $connections = clients_connection_load_all();
  if (isset($connections[$form_state['values']['name']]) && $form_state['values']['name'] != $old_connection->name) {
    form_set_error('name', t('This connection name is already taken.'));
  }

  // TODO: consider cleaning up the following code to test on form submit:
  /*
  $connection = new stdClass;
  $connection->name = $form['name']['#value'];
  $connection->endpoint = $form['endpoint']['#value'];
  $connection->domain = $form['configuration']['domain']['#value'];
  $connection->servicekey = $form['configuration']['servicekey']['#value'];
  $connection->username = $form['configuration']['username']['#value'];
  $connection->password = $form['configuration']['password']['#value'];

  $testconnect = ClientsServicesDrupal::connect($connection);
  if(!is_array($testconnect) || !isset($testconnect['sessid'])) {
      form_set_error('endpoint', "Couldn't connect");
  } else {
      $testuser = ClientsServicesDrupal::getUser($connection);
      if(!is_array($testuser) || !isset($testuser['sessid'])) {
          form_set_error('username', isset($testuser->message) ? $testuser->message : "Couldn't log in");
      }
  }
  */
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
 * @see clients_connection_form_validate()
 */
function clients_connection_form_submit($form, &$form_state) {
  //dsm($form);
  //dsm($form_state);
  $class = $form['#connection_class'];

  $old_connection = $form_state['values']['old_connection'];

  // Use call_user_func_array() so form state can be passed by reference.
  call_user_func_array(array($class, 'connectionSettingsForm_submit'), array($form, &$form_state));

  // Common actions for all forms.

  $connection = $form_state['values'];
  // Connections with no configuration nonetheless need the array.
  if (!isset($connection['configuration'])) {
    $connection['configuration'] = array();
  }
  // Add in the cid, if there was one originally.
  // Check whether we're editing or adding a new connection.
  // We could use $old_connection->new to determine whether this is an insert
  // or an update, but that's probably not going to work come CTools, as a
  // connection may not be 'new' but may not have a cid if it's from code.
  if (isset($old_connection->cid)) {
    $connection['cid'] = $old_connection->cid;
    drupal_write_record('clients_connections', $connection, 'cid');
  }
  else {
    drupal_write_record('clients_connections', $connection);
  }

  drupal_set_message(t('Connection saved.'));
  $form_state['redirect'] = 'admin/build/clients/connections/' . $connection['name'];
}

/**
 * Page callback to view a single connection.
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

  return theme_table(array(), $rows);
}


/**
 * Page callback to test a connection.
 *
 * @param $connection
 *  A loaded connection.
 */
function clients_connection_test_form(&$form_state, $connection) {
  $form['#connection'] = $connection;

  $form['connection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Connection details'),
  );
  $items[] = t('Type') . ': ' . $connection->type;
  $items[] = t('Name') . ': ' . check_plain($connection->name);
  $items[] = t('Endpoint') . ': ' . $connection->endpoint;
  $form['connection']['details'] = array(
    '#value' => theme('item_list', $items),
  );

  // Buttons. These are themed into a vertical list with the description text alongside each one.
  $form['buttons'] = array(
    //'#theme' => 'uprpc_manual_form_button',
    '#tree' => TRUE,
  );

  // Get the core buttons provided by the connection class.
  $buttons = $connection->getTestOperations($form_state, $connection->cid);

  // Allow applications that use this connection to add their own test buttons.
  drupal_alter('client_connection_test_buttons', $buttons, $form_state, $connection->cid);

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
