<?php

/**
 * @file
 * Hooks provided by the Clients module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Inform Clients about connection types.
 *
 * @return array
 *   An array of information on the connection types implemented by a module,
 *   keyed by the machine-readable name for the type.
 *   Each type is itself an array, with following keys:
 *     'label': the human-readable label.
 */
function hook_clients_connection_type_info() {
  return array(
    'my_client' => array(
      'label'  => t('My Client Type'),
    ),
  );
}

/**
 * Add or alter connection actions to test a connection.
 *
 * @param $buttons
 *  The array of buttons that the client connection type provides.
 *  This is an array of FormAPI elements, so is keyed by form element ID. If you
 *  add buttons here, prefix them with your module name to avoid clashes.
 *  The following FormAPI properties are relevant:
 *    '#type': This should be either 'submit' or 'fieldset'. Use the first if you
 *    want just a button; it will be wrapped up in a fieldset for you.
 *    Use fieldset if you need to provide extra fields. Your fieldset must then
 *    contain a submit button, with id 'button'.
 *  In addition to normal FormAPI properties, the following may also be used:
 *    '#description': This is added as description text to the fieldset. Set 
 *    this on the button element in all cases.
 *    '#action_type': (optional) One of 'function' or 'method', to indicate whether the 
 *    submit and validate callbacks are methods on the current connection 
 *    object or just regular functions. Default is 'function'.
 *    '#action_submit': submit handler for the button. This can be either the 
 *    name of a method on the connection object, or a function name, depending 
 *    on the value of '#action_type'.
 *    This should have the signature my_test_submit(&$button_form_values), where
 *    $button_form_values is the data from the form values tree specific to the 
 *    button or fieldset.
 *    Your submit handler should simply return its results, and the connection
 *    testing form will take care of displaying them beneath the button after
 *    submission.
 *    '#action_validate' (optional): validate handler for the button. Works the
 *    same as '#action_submit'.
 * 
 * @param $form_state
 *  The $form_state parameter from the connection test form. You can use this
 *  to populate default values.
 * @param $cid
 *  The current connection id. Use this to determine whether to add any buttons;
 *  for example, by comparing it to the connection your module is set up to use.
 */
function hook_client_connection_test_buttons_alter($buttons, $form_state, $cid) {
  $buttons['my_module_test'] = array(
    '#value' => 'Test something to do with my module',
    '#type' => 'submit',
    '#action_type' => 'function',
    '#action_submit' => 'my_module_test_submit',
    '#description' => t('Test something on this connection specific to my module.'),
  );
}
