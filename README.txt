Connections
===========

Clients module provides a simple UI for creating, editing, and testing
connections to remote sites.

Connections are exportable with CTools and thus can be added to Features.

Requirements
============

Clients requires the following modules:

- Autoload
- CTools 6.x-1.x-dev.
  If you'd rather not run a development version, these are the patches to
  6.x-1.0 that Clients needs:
  - http://drupal.org/node/1094014
  - http://drupal.org/node/1146604

Using the Clients API
=====================

The Clients connection object lets you call methods on remote sites very
simply, without having to deal with tokens, keys, and all that sort of
stuff.

Once a connection is defined in the admin UI, you can call a remote method on it
like this:

  // 'my_connection' is the machine name of the connection.
  $result = clients_connection_call('my_connection', 'method.name', $param1, $param2, $param_etc);

So for example, to load a node from a remote Drupal site, do:
  
  $node = clients_connection_call('my_connection', 'node.get', $remote_nid);
  // Note that the $node will be an array.

If you need to make several calls, you can use the connection object yourself:

  $connection = clients_connection_load('my_connection');
  $result = $connection->callMethod('method.name', $param1, $param2, $param_etc);


Defining Connection types
=========================

To define your own connection type, you need:

- an implementation of hook_clients_connection_type_info().
- a class definition for your connection type. This should be named
  'clients_connection_YOUR_TYPE_ID' and implement the following methods:
  - connectionSettingsForm(), which should return a FormAPI array for your
    connection type's edit form.
  - connectionSettingsForm_submit(), which should provide any processing of
    the form specific to your connection type.
  - callMethodArray(), which should call a remote method and return the result.

