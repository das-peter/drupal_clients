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
  
  $connections = array();
  // TODO: clients_get_connections and clients_connection_load make the same queries!!!
  foreach (array_keys(clients_get_connections()) as $cid) {
    $connection = clients_connection_load($cid);      
    $connections[] = array(
      'name' => l($connection->name, "admin/build/clients/connections/$cid/view"),
      'type' => $connection->type,
      'endpoint' => $connection->endpoint,
      'edit' => l('edit', "admin/build/clients/connections/$cid/edit"),
      'test' => l('test', "admin/build/clients/connections/$cid/test"),
      'delete' => l('delete', "admin/build/clients/connections/$cid/delete"),
    );
  }
  
  // Ensure the table has a row if there are no connections at all.
  if (!count($connections)) {
    $connections[] = array(
      array(
        'data' => t('No connections defined yet.'),
        'colspan' => '5',
      ),
    );
  }
  
  $headers = array('Name', 'Type', 'Endpoint', array('data' => 'Operations', 'colspan' => 3));
  $output .= theme_table($headers, $connections);
  
  // Add a list of connection types that can be created.
  foreach ($connection_types as $type => $type_data) {
    //dsm($type_data);
    $items[] = l(t('Add @type connection', array('@type' => $type_data['label'])), 'admin/build/clients/connections/add/' . $type);
  }
  $output .= theme('item_list', $items);
  
  return $output;
}

/**
 * Form builder for adding a connection.
 *
 * Gets the class of the connection from the data in the form and calls the
 * connectionSettingsForm() method on the class to build the form.
 * This allows different elements for different connection types. 
 *
 * @param $type
 *  The machine name of a connection type.
 *
 * @see clients_connection_form_submit() 
 */
function clients_connection_add(&$form_state, $type) {
  $form = array();
  $connection_types = clients_get_connection_types();
  //dsm($connection_types);
  
  if (isset($connection_types[$type])) {
    $class = 'clients_connection_' . $type;

    // When 5.3 is the norm, we can do:
    //$form = $class::connectionSettingsForm($form_state);
    $form['#connection_type'] = $type;
    $form['#connection_class'] = $class;
    
    $form['type'] = array(
      '#type' => 'textfield',
      '#title' => t('Connection type'),
      '#description' => t('The type of connection to create. May not be changed.'),
      '#value' => $type,
      '#size' => 50,
      '#disabled' => TRUE,
    );    

    // When PHP 5.3 is the norm, we can just say $class::$method().
    $form += call_user_func(array($class, 'connectionSettingsForm'), $form_state, $type);
    
    $form['buttons']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Add connection'),
    );    
    
    // Add our submit handler common with the edit form.
    $form['#submit'] = array('clients_connection_form_submit');
    
    return $form;  
  }  
  else {
    return drupal_not_found();
  }
}

/**
 * Validator handler for the add connection form.
 */
function clients_connection_add_validate($form, &$form_state) {
  // Names must be unique.
  $cid = db_result(db_query("SELECT cid FROM {clients_connections} WHERE name = '%s'", $form_state['values']['name']));
  if ($cid) {
    form_set_error('name', 'A service by this name already exists!');
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
 * Form builder for editing a connection.
 *
 * @param $cid
 *  The id of a connection.
 *
 * @see clients_connection_form_submit() 
 */
function clients_connection_edit(&$form_state, $cid) {
  $connection = clients_connection_load($cid);
  $type = $connection->type;
  $class = 'clients_connection_' . $type;
  
  //dsm($connection);
  
  $form['type'] = array(
    '#type' => 'textfield',
    '#title' => t('Connection type'),
    '#description' => t('The type of this connection. May not be changed.'),
    '#value' => $type,
    '#size' => 50,
    '#disabled' => TRUE,
  );

  $form += call_user_func(array($class, 'connectionSettingsForm'), $form_state, $type, $connection);
  
  $form['#connection_type'] = $type;
  $form['#connection_class'] = $class;
  
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save connection'),
  );    
  
  // Add our submit handler common with the add form.
  $form['#submit'] = array('clients_connection_form_submit');    
  
  return $form;
}

/**
 * Common submit handler for connection add and edit forms.
 *
 * Gets the class of the connection from the data in the form and calls the
 * connectionSettingsForm_submit() method on the class.
 * This allows different behaviour for different connection types.
 *
 * @see clients_connection_add()
 * @see clients_connection_edit()
 */
function clients_connection_form_submit($form, &$form_state) {  
  //dsm($form);
  //dsm($form_state);
  $class = $form['#connection_class'];
  
  // Use call_user_func_array() so form state can be passed by reference.
  call_user_func_array(array($class, 'connectionSettingsForm_submit'), array($form, &$form_state));
  
  // Common actions for all forms.
  // Presence of the cid tells us whether we're editing or adding a new connection.
  $new = !isset($form_state['values']['cid']);  
  if ($new) {
    drupal_write_record('clients_connections', $form_state['values']);
  }
  else {
    drupal_write_record('clients_connections', $form_state['values'], 'cid');
  }

  drupal_set_message(t('Connection saved.'));
  $form_state['redirect'] = 'admin/build/clients/connections';  
}

/**
 * Page callback to view a single connection.
 *
 * @param $cid
 *  The id of a connection.
 */
function clients_connection_view($cid) {
  // Load the connection.
  $connection = clients_connection_load($cid);
  
  // Build summary table.
  $rows = array();
  $rows[] = array('Name', check_plain($connection->name));
  $rows[] = array('Type', $connection->type);
  $rows[] = array('Endpoint', check_plain($connection->endpoint));
  foreach ($connection->configuration as $label => $val) {
    if (is_array($val)) {
      $val = implode(', ', $val); // needs to be recursive?
    }
    $rows[] = array(ucfirst(check_plain($label)), nl2br(check_plain($val)));
    
  }

  return theme_table(array(), $rows);
}


/**
 * Page callback to test a connection.
 *
 * @param $cid
 *  The id of a connection.
 */
function clients_connection_test_form(&$form_state, $cid) {  
  $connection = clients_get_connection($cid);
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
  $buttons = $connection->getTestOperations($form_state, $cid);
  
  // Allow applications that use this connection to add their own test buttons.
  drupal_alter('client_connection_test_buttons', $buttons, $form_state, $cid);
  
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
      $title = $form['buttons'][$button_id]['#value'];
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
