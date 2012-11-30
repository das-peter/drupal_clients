<?php
/**
 * @file clients.resource.admin.inc
 * Page callbacks relating to client resource admin.
 */

/**
 * Resource form.
 */
function clients_resource_form($form, &$form_state, $resource, $op = 'edit') {
  $form = array();

  dsm($resource);

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Resource label'),
    '#default_value' => isset($resource->label) ? $resource->label : '',
    '#size' => 50,
    '#maxlength' => 100,
    '#description' => t('The human-readable name for the resource.'),
    '#required' => TRUE,
  );

  // Machine-readable type name.
  $form['name'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($resource->name) ? $resource->name : '',
    '#maxlength' => 32,
    '#disabled' => ($op == 'clone'),
    '#machine_name' => array(
      'exists' => 'clients_resource_load',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this resource. It must only contain lowercase letters, numbers, and underscores.'),
  );

  /*
  // add this as markup for info?
  $form['type'] = array(
    '#type' => 'textfield',
    '#title' => t('Connection type'),
    '#description' => t('The type of this connection. May not be changed.'),
    '#value' => $resource->type,
    '#size' => 50,
    '#disabled' => TRUE,
  );
  */

  $form['component'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($resource->component) ? $resource->component : '',
    '#maxlength' => 32,
    '#description' => t('The component to provide.'),
  );
  // TODO: subclasses to do work here!

  $options = clients_connections_select_options();
  $form['connection'] = array(
    '#type' => 'select',
    '#title' => t('Connection'),
    '#options' => $options,
    '#default_value' => isset($resource->connection) ? $resource->connection : '',
    '#description' => t('The connection to use for this resource.'),
  );

  // Container for all form elements whose values should be serialized to the
  // configuration array.
  // Not a fieldset by default, but connection classes may choose to do this.
  $form['configuration'] = array(
    '#tree' => TRUE,
  );

  // Allow the connection class to make additions to the form. We don't use
  // the node_form() pattern because typically a connection class will want to
  // change the 'endpoint' element to add custom help text.
  $form = $resource->connectionSettingsForm($form, $form_state);

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save resource'),
  );

  return $form;
}

function clients_resource_form_submit($form, &$form_state) {
  $resource = entity_ui_form_submit_build_entity($form, $form_state);

  // Save and go back.
  $resource->save();

  drupal_set_message(t('Resource saved.'));

  $form_state['redirect'] = 'admin/structure/clients/resources';
}
