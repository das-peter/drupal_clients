<?php

/**
 * @file
 * Default hook definitions. This code is generated with Features module but it
 * has been tweaked manually. Do not attempt to reexport.
 */

/**
 * Implementation of hook_ctools_plugin_api().
 */
function clients_feeds_ctools_plugin_api() {
  module_load_include('inc', 'clients_feeds', 'clients_feeds.defaults');
  $args = func_get_args();
  return call_user_func_array('_clients_feeds_ctools_plugin_api', $args);
}

/**
 * Implementation of hook_feeds_importer_default().
 */
function clients_feeds_feeds_importer_default() {
  module_load_include('inc', 'clients_feeds', 'clients_feeds.defaults');
  $args = func_get_args();
  return call_user_func_array('_clients_feeds_feeds_importer_default', $args);
}
