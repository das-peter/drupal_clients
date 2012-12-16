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
 * Additions to hook_entity_info().
 *
 * These support the various controllers that provide functionality for
 * entities that are used as handlers.
 *
 * - 'admin ui': This is defined by EntityAPI, but we add the following keys:
 *    - 'access permission': A permission to use for access to all of this
 *      entity's admin UI. Using this means that the entity does not need an
 *      access callback.
 *    - 'types callback': The name of a callback function that gives a list of
 *      types of handlers. The keys of the list should be the machine names; the
 *      values are ignored.
 * - 'factory': An array of information on how to build handler objects.
 *    - 'class prefix': The prefix to apply to the handler type to make the name
 *      of the class to use.
 *    - 'broken class': The class to use when the class for a handler cannot be
 *      found, for example, if a module has been disabled.
 */
function clients_hook_entity_info() {

}

/**
 * Inform Clients about connection types.
 *
 * @return array
 *   An array of information on the connection types implemented by a module,
 *   keyed by the machine-readable name for the type.
 *   Each type is itself an array, with following keys:
 *     'label': the human-readable label.
 *     'description': (optional) A more detailed description of the type.
 *     'tests': (optional) An array of test classes that this connection type
 *        can use for the testing UI. The keys should be the test IDs, and the
 *        values the name of the test class, which should implement
 *        ClientsConnectionTestingInterface.
 */
function hook_clients_connection_type_info() {
  return array(
    'my_client' => array(
      'label'  => t('My Client Type'),
      'tests' => array(
        'connect' => 'MyClientTypeTestConnection',
      ),
    ),
  );
}

/**
 * Alter Clients connection types definitions.
 *
 * @param $connection_types
 *   An array of information on the connection types as returned by
 *   hook_clients_connection_type_info().
 */
function hook_clients_connection_type_info_alter($connection_types) {
}

/**
 * Add or alter connection tests.
 *
 * @param $tests
 *  The array of tests as returned by hook_clients_connection_type_info(). May
 *  be empty.
 * @param $connection
 *  The current connection. Use this to determine whether to add any tests;
 *  for example, by comparing it to the connection your module is set up to use.
 */
function hook_client_connection_tests_alter(&$tests, $connection) {
  if ($connection->type == 'foobar') {
    $tests['my_test'] = 'MyTestClass';
  }
}

/**
 * Inform Clients about resource types.
 *
 * @return array
 *   An array of information on the resource types implemented by a module,
 *   keyed by the machine-readable name for the type.
 *   Each type is itself an array, with following keys:
 *     'label': the human-readable label.
 *     'description': (optional) A more detailed description of the type.
 */
function hook_clients_resource_type_info() {
  return array(
    'my_resource' => array(
      'label'  => t('My resource type'),
    ),
  );
}


/**
 * Define default client connections.
 *
 * Used via EntityAPI exportables.
 */
function hook_clients_default_connections() {

}

/**
 * Define default client resources.
 *
 * Used via EntityAPI exportables.
 */
function hook_clients_default_resources() {

}
