<?php
// $Id$

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
