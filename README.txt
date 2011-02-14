// $Id$

Using the Clients API
=====================

The Clients connection object lets you call methods on remote sites very
simply, without having to deal with tokens, keys, and all that sort of
stuff.

Once a connection is defined in the admin UI, you can call a remote method on it
like this:

  // $connection_id is the ID of the connection.
  $connection = clients_get_connection($connection_id);
  $result = $connection->callMethod('method.name', $param1, $param2, $param_etc);
  
Example: getting a node from a remote site:

  $connection = clients_get_connection($drupal_site_connection_id);
  $node = $connection->callMethod('node.get', $remote_nid);
  // Note that the $node will be an array.

Defining Connection types
=========================

To define your own connection type, you need:

- an implementation of hook_clients_connection_type_info().
- a class definition for your connection type, with the following methods:
  - connectionSettingsForm(), which should return a FormAPI array for your
    connection type's edit form.
  - connectionSettingsForm_submit(), which should provide any processing of
    the form specific to your connection type.
  - callMethod(), which should call a remote method and return the result.

