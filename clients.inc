<?php
/**
 * @file
 * Contains basic classes for client connections.
 */

/**
 * Base class for client connections.
 */
abstract class clients_connection_base {
  /**
   * The machine name of the connection.
   */
  public $name;

  /**
   * The connection id. Only set if this is stored in the database.
   */
  public $cid;

  /**
   * The URL this connection connects to.
   */
  public $endpoint;

  /**
   * An array of further configuration options.
   */
  public $configuration;

  // ============================================ Connection UI

  /**
   * Format the connection's endpoint as a link.
   *
   * @param $url
   *  The connection's endpoint.
   *
   * @return
   *  The string to display in the admin UI. Subclasses may format this as a
   *  link to the remote site.
   */
  function formatEndpoint($url) {
    return $url;
  }

  /**
   * Extra form elements specific to a class's edit form.
   *
   * This is the same pattern as node_form() -- just ignore the object behind
   * the curtain ;)
   *
   * This (so far) is common to all versions of Drupal Services.
   *
   * @param $form_state
   *  The form state from the main form, which you probably don't need anyway.
   *
   * @return
   *  A FormAPI form array. This will be merged in with basic data and the
   *  submit button added.
   *
   * @see clients_connection_form()
   * @see clients_connection_form()
   * @see clients_connection_form_submit()
   */
  function connectionSettingsForm(&$form_state) {
    $form = array();
    return $form;
  }

  /**
   * Submit handler for saving/updating connections of this class.
   *
   * @see clients_connection_form_submit().
   */
  static function connectionSettingsForm_submit($form, &$form_state) {
    // Base class does nothing; saving of the connection is handled by the
    // 'real' FormAPI submit handler.
  }

  /**
   * Provide buttons for the connection testing page.
   */
  function getTestOperations() {
    return array();
  }

  // ============================================ Constructor.

  /**
   * Constructor method.
   *
   * @param $object
   *  An object of class stdClass returned from CTools.
   */
  function __construct($object) {
    // Lump all data unto the object...
    foreach ((array) $object as $field => $value) {
      $this->$field = $value;
    }

    // Connections defined in code are already unserialized.
    if (!is_array($object->configuration)) {
      $this->configuration = unserialize($object->configuration);
    }

    return $this;
  }

  // ============================================ Connection API.

  /**
   * Call a remote method.
   *
   * This is a wrapper around callMethodArray that gives the convenience of
   * being able to pass method name and parameters as one flat list, and hence
   * is the main API for connection objects.
   *
   * @param $method
   *  The name of the remote method to call.
   * @param ...
   *  All other parameters are passed to the remote method.
   *
   * @return
   *  Whatever is returned from the remote site.
   */
  function callMethod($method) {
    // Get all the arguments this function has been passed.
    $function_args  = func_get_args();
    // Slice out the ones that are arguments to the method call: everything past
    // the 1st argument.
    $method_params  = array_slice($function_args, 1);

    return $this->callMethodArray($method, $method_params);
  }

  /**
   * Call a remote method with an array of parameters.
   *
   * This is intended for internal use from callMethod() and
   * clients_connection_call().
   * If you need to call a method on given connection object, use callMethod
   * which has a nicer form.
   *
   * @param $method
   *  The name of the remote method to call.
   * @param $method_params
   *  An array of parameters to passed to the remote method.
   *
   * @return
   *  Whatever is returned from the remote site.
   */
  function callMethodArray($method, $method_params = array()) {
    // Up to subclasses to override this to do something.
  }
}

/**
 * Connection class for broken connections.
 */
class clients_connection_broken extends clients_connection_base {
  /**
   * Constructor method for broken connections.
   *
   * This expects the originator to have set a readable message in
   * $object->broken_message.
   *
   * @param $object
   *  An object of class stdClass returned from CTools.
   */
  function __construct($object) {
    // Call the base class to set the connection properties.
    parent::__construct($object);

    // Add some output for the view connection page.
    $this->configuration['WARNING'] = t("This connection is broken.");
    $this->configuration['broken message'] = $object->broken_message;

    $connection_path = 'admin/build/clients/connections/' . $this->name;
    if ($_GET['q'] == $connection_path) {
      $message = t('This connection is broken.');
    }
    else {
      $message = t('A connection is broken. See its <a href="@url">view page</a> for further details.', array(
        '@url' => url($connection_path),
      ));
    }

    drupal_set_message($message, 'warning');
  }
}
