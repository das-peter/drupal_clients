<?php
/**
 * @file
 * Contains classes for Client connections handlers.
 */

/**
 * General Drupal client class.
 *
 * This should connect to Drupal 7 services. Which are still at the RC stage...
 * so it's largely an abstract class for the moment.
 */
class clients_connection_drupal_services extends clients_connection_base {
  // ============================================ Connection form methods.

  /**
   * Form builder for adding or editing connections of this class.
   *
   * Static function, call without an object.
   *
   * This (so far) is common to all versions of Drupal Services.
   *
   * @param $type
   *  The type of the connection.
   * @param $connection
   * (optional) The connection object, if this is an edit.
   *
   * @return
   *  A FormAPI form array. This will be merged in with basic data and the
   *  submit button added.
   *
   * @see clients_connection_add()
   * @see clients_connection_edit()
   */
  static function connectionSettingsForm(&$form_state, $type, $connection = NULL) {
    $form = array();

    if (is_null($connection)) {
      $new = TRUE;
      $connection_types = clients_get_connection_types();
      drupal_set_title(t('Add @type connection', array(
        '@type' => $connection_types[$type]['label'],
      )));
    }
    else {
      $form['cid'] = array(
        '#type' => 'value',
        '#value' => $connection->cid,
      );
    }

    $form['endpoint'] = array(
      '#type' => 'textfield',
      '#title' => t('Connection endpoint'),
      '#default_value' => isset($connection) ? $connection->endpoint : '',
      '#size' => 50,
      '#maxlength' => 100,
      '#description' => t('Remote service URL e.g. http://mysite.com/services/xmlrpc'),
      '#required' => TRUE,
    );

    $form['configuration'] = array(
      '#type' => 'fieldset',
      '#title' => t('Configuration'),
      '#collapsible' => FALSE,
      '#tree' => TRUE,
    );

    $form['configuration']['domain'] = array(
      '#type' => 'textfield',
      '#title' => t('Domain'),
      '#default_value' => isset($connection) ? $connection->configuration['domain'] : '',
      '#size' => 50,
      '#maxlength' => 100,
      '#description' => t('This should be same as the \'Domain\' field used by the Services authentication key on the server you are connecting to.'),
      '#required' => TRUE,
    );
    $form['configuration']['servicekey'] = array(
      '#type' => 'textfield',
      '#title' => t('Service key'),
      '#default_value' => isset($connection) ? $connection->configuration['servicekey'] : '',
      '#size' => 50,
      '#maxlength' => 40,
      '#attributes' => array('autocomplete' => 'off'),
      '#description' => t('This should be same as the \'Key\' field used by the Services authentication key on the server you are connecting to.'),
      '#required' => TRUE,
    );
    $form['configuration']['username'] = array(
      '#type' => 'textfield',
      '#title' => t('Service username'),
      '#default_value' => isset($connection) ? $connection->configuration['username'] : '',
      '#size' => 30,
      '#maxlength' => 60,
      '#attributes' => array('autocomplete' => 'off'),
      '#description' => t('This should be same as the username on the server you are connecting to.'),
      '#required' => TRUE,
    );

    $password_desc = isset($connection)
      ? t('This should be same as the password on the server you are connecting to. Leave blank unless you need to change this.')
      : 'This should be same as the password on the server you are connecting to.';
    $form['configuration']['password'] = array(
      '#type' => 'password',
      '#title' => t('Service password'),
      '#size' => 30,
      '#maxlength' => 60,
      '#attributes' => array('autocomplete' => 'off'),
      '#description' => $password_desc,
      '#required' => $new,
    );

    return $form;
  }

  /**
   * Submit handler for saving/updating connections of this class.
   *
   * @see clients_connection_form_submit().
   */
  static function connectionSettingsForm_submit($form, &$form_state) {
    // Presence of the cid tells us whether we're editing or adding a new connection.
    $new = !isset($form_state['values']['cid']);

    if ($new) {
      $form_state['values']['configuration']['password'] = clients_drupal_encrypt($form_state['values']['configuration']['password']);
    }
    else {
      // Prepare password for serialized storage
      if (empty($form_state['values']['configuration']['password'])) {
        // Need to load connection and set password to original if blank.
        $original = clients_connection_load((int) $form_state['values']['cid']);
        $form_state['values']['configuration']['password'] = $original->configuration['password'];
      }
      $form_state['values']['configuration']['password'] = clients_drupal_encrypt($form_state['values']['configuration']['password']);
    }
  }

  // ============================================ Constructor.

  /**
   * Constructor method.
   *
   * @param $connection_data
   *  An object containing connection data, as returned from clients_connection_load().
   */
  function __construct($row) {
    // Call the base class to set the connection properties.
    parent::__construct($row);

    // Decrypt the password.
    $this->configuration['password'] = clients_drupal_decrypt($this->configuration['password']);
  }

  // ============================================ Connection API.

  /**
   * Call a remote method.
   *
   * @param $method
   *  The name of the remote method to call.
   * @param
   *  All other parameters are passed to the remote method.
   *  Note that the D5 version of Services does not seem to respect optional parameters; you
   *  should pass in defaults (eg an empty string or 0) instead of omitting a parameter.
   *
   * @return
   *  Whatever is returned from the remote site.
   */
  function callMethod($method) {
    // TODO: Needs to be written for Services D7.
    //dsm($method);
  }

  // ===================================== old static stuff

  /**
   * Use for testing
   */
  static public function connect($connection) {
    $session = xmlrpc($connection->endpoint, 'system.connect');
    if ($session === FALSE) {

      return xmlrpc_error(); // null for services 2...
    }
    return $session;
  }

  /**
   * Prepares a hashed token for the service, based on current time,
   * the required service and config values; serviceKey and serviceDomain
   *t
   * @param $connection
   *   stdClass: A service connection as returned by Clients::load()
   * @param $serviceMethod
   *   string: Name of service method to access
   *
   * @return
   *   array a valid token
   */
  static public function getToken($connection, $serviceMethod) {
    $timestamp = (string)time();
    $nonce = uniqid();
    $hashParameters = array($timestamp, $connection->configuration['domain'], $nonce, $serviceMethod);
    $hash = hash_hmac("sha256", implode(';', $hashParameters), $connection->configuration['servicekey']);

    return array(
      'hash'      => $hash,
      'domain'    => $connection->configuration['domain'],
      'timestamp' => $timestamp,
      'nonce'     => $nonce
    );
  }

  /**
   * Connects to Drupal Services and logs in the user provided in the config.
   * Returns a session for the user.
   * @todo needs error catching in case service is down
   *
   * @return
   *   array
   */
  static public function getUser($connection) {
    $session = self::connect($connection);

    if ($session->is_error == TRUE) {
      drupal_set_message('There was an error connecting to the service.');
      return;
    }

    $userToken = self::getToken($connection, 'user.login');

    $user = xmlrpc(
      $connection->endpoint,
      'user.login',
      $userToken['hash'],
      $userToken['domain'],
      $userToken['timestamp'],
      $userToken['nonce'],
      $session['sessid'],
      $connection->configuration['username'],
      $connection->configuration['password']
    );

    if ($user === FALSE) {
      return xmlrpc_error();
    }
    return $user;
  }

  /**
   * Gets raw data from service call
   */
  static protected function fetch($connection, $resource, $user) {
      $cacheid = md5($connection->name . implode($resource->configuration['options']));
      // user is stdClass if xmlrpc_error()...
      if (!is_array($user) || !isset($user['sessid'])) {
        // @todo watchdog
        drupal_set_message($user->message, 'error');
        return;
      }
      $token = self::getToken($connection, $resource->configuration['options']['method']);

      // catch empty arguments
      $arguments = array_values($resource->configuration['options']['arguments']);
      $arguments = trim($arguments[0]) != '' ? $arguments : array();

      $result = parent::doCall(
        'xmlrpc',
        $cacheid,
        $connection->endpoint,
        $resource->configuration['options']['method'],
        $token['hash'],
        $token['domain'],
        $token['timestamp'],
        $token['nonce'],
        $user['sessid'],
        $resource->configuration['options']['view'],
        NULL, // services 2 beta1 display id - default
        $arguments,
        (int)$resource->configuration['options']['offset'],
        (int)$resource->configuration['options']['limit']
      );

      return $result;
  }

  /**
   * Executes call and processes data
   */
  static public function call($connection, $resource) {
    if ($resource->configuration['options']['method'] == 'views.get') {

      $user = self::getUser($connection);
      // gets raw result
      $result = self::fetch($connection, $resource, $user);
      // needs some post-processing
      $processed_result = new stdClass;
      $processed_result->created = $result->created;
      $processed_result->data = array();

      foreach ($result->data as $item) {

        foreach ($item as $k => $v) { // handle CCK field data structure
          if (strpos($k, 'field_') !== FALSE) {
            $item[$k] = $v[0]['value'];
          }
        }

        // nid will interfere with local nids in client
        $item['remote_nid'] = $item['nid'];
        unset($item['nid']);
        // remote taxonomy is not understood locally so flatten to RSS-style bag of tags (TODO: develop this to preserve vocabs)
        $tags = array();
        if (isset($item['taxonomy'])) {
          foreach ((array)$item['taxonomy'] as $term) {
              $tags = $term['name'];
          }
          unset($item['taxonomy']);
        }
        $item['tags'] = $tags;

        $processed_result->data[] = $item;
      }
      return $processed_result;
    } // else method not supported yet

  }

}

/**
 * Drupal client for services on a Drupal 6 site for Services 6.x-2.x.
 *
 * Developed against Services 6.x-2.4.
 */
class clients_connection_drupal_services_6_2 extends clients_connection_drupal_services {
  /**
   * Call a remote method.
   *
   * @param $method
   *  The name of the remote method to call.
   * @param
   *  All other parameters are passed to the remote method.
   *
   * @return
   *  Whatever is returned from the remote site.
   */
  function callMethod($method) {
    // If HTTP requests are enabled, report the error and do nothing.
    // (Cribbed from Content distribution module.)
    if (variable_get('drupal_http_request_fails', FALSE) == TRUE) {
      drupal_set_message(t('Drupal is unable to make HTTP requests. Please reset the HTTP request status.'), 'error', FALSE);
      watchdog('integration', 'Drupal is unable to make HTTP requests. Please reset the HTTP request status.', array(), WATCHDOG_CRITICAL);
      return;
    }

    $config = $this->configuration;
    $endpoint = $this->endpoint;
    $api_key = $this->configuration['servicekey'];

    // Connect to the remote system service to get an initial session id to log in with.
    $connect = xmlrpc($this->endpoint, 'system.connect');
    $session_id = $connect['sessid'];

    // We may want to call only system.connect for testing purposes.
    if ($method == 'system.connect') {
      return $connect;
    }

    // Log in
    // Get the API key-related arguments.
    $key_args = $this->xmlrpc_key_args('user.login');
    //dsm($key_args);

    // Build the array of connection arguments we need to log in.
    $username = $this->configuration['username'];
    $password = $this->configuration['password'];
    $login_args = array_merge(
      array($this->endpoint, 'user.login'), // endpoint, method
      $key_args,                            // hash, domain_name, domain_time_stamp, nonce
      array($session_id),                   // sessid
      array($username, $password)           // username, password
    );

    // Call the xmlrpc method with our array of arguments. This accounts for
    // whether we use a key or not, and the extra parameters to pass to the method.
    $login = call_user_func_array('xmlrpc', $login_args);
    $login_session_id = $login['sessid'];

    // If the requested method is user.login, we're done.
    if ($method == 'user.login') {
      return $login;
    }

    // Get all the arguments this function has been passed.
    $function_args  = func_get_args();
    // Slice out the ones that are arguments to the method call: everything past
    // the 1st argument.
    $method_args    = array_slice($function_args, 1);

    // Get the API key-related arguments.
    $key_args = $this->xmlrpc_key_args($method);

    // Build the array of connection arguments for the method we want to call.
    $xmlrpc_args = array_merge(
      array($this->endpoint, $method),
      $key_args,
      array($login_session_id),
      $method_args
    );

    // Call the xmlrpc method with our array of arguments.
    $result = call_user_func_array('xmlrpc', $xmlrpc_args);

    if ($result === FALSE) {
      //dsm('error');
      return xmlrpc_error();
    }

    return $result;
  }

  /**
   * Helper function to get key-related method arguments for the XMLRPC call.
   *
   * TODO: merge with getToken.
   */
  function xmlrpc_key_args($method) {
    $api_key = $this->configuration['servicekey'];

    // Build the API key arguments - if no key supplied supplied, presume not required
    if ($api_key != '') {
      //use api key to get a hash code for the service.
      $timestamp = (string) strtotime("now");
      // Note that the domain -- at least for Services 5 and 6.x-2.x -- is a
      // purely arbitrary string more akin to a username.
      // See http://drupal.org/node/821700 for background.
      $domain = $this->configuration['domain'];
      /*
      if (!strlen($domain)) {
        $domain = $_SERVER['SERVER_NAME'];
        if ($_SERVER['SERVER_PORT'] != 80) {
          $domain .= ':' . $_SERVER['SERVER_PORT'];
        }
      }
      */
      $nonce = uniqid();
      $hash_parameters = array(
        $timestamp,
        $domain,
        $nonce,
        $method,
      );
      $hash = hash_hmac("sha256", implode(';', $hash_parameters), $api_key);

      $key_args = array($hash, $domain, $timestamp, $nonce);
    }
    else {
      $key_args = array();
    }

    return $key_args;
  }

  /**
   * Provide buttons for the connection testing page.
   *
   * @param $form_state
   *  This is passed in so you can set defaults based on user input.
   */
  function getTestOperations($form_state, $cid) {
    $buttons['connect'] = array(
      '#value' => 'Test connection',
      '#type' => 'submit',
      //'#name' => 'connect', // wtf does this do?
      '#action_type' => 'method',
      '#action_submit' => 'testConnectionConnect',
      '#description' => t('Test the connection settings by calling system.connect on the remote server.'),
    );
    $buttons['login'] = array(
      '#value' => 'Test user login',
      '#type' => 'submit',
      //'#name' => 'login',
      '#action_type' => 'method',
      '#action_submit' => 'testConnectionLogin',
      '#description' => t('Test the remote user settings and by calling user.login on the remote server.'),
    );
    $buttons['node_load'] = array(
      '#type' => 'fieldset',
    );
    $buttons['node_load']['nid'] = array(
      '#type' => 'textfield',
      '#title' => 'Node ID',
      '#size' => 6,
      '#default_value' => isset($form_state['values']['buttons']['node_load']['nid']) ? $form_state['values']['buttons']['node_load']['nid'] : NULL,
    );
    $buttons['node_load']['button'] = array(
      '#value' => 'Test node retrieval',
      '#type' => 'submit',
      //'#name' => 'login',
      // TODO: tidy up these method names!
      '#action_type' => 'method',
      '#action_submit' => 'testConnectionNodeLoad',
      '#action_validate' => 'testConnectionNodeLoadValidate',
      '#description' => t('Attempt to load a remote node.'),

    );
    return $buttons;
  }

  /**
   * Connection test button handler: basic connection.
   *
   * Connection test handlers should return the raw data they got back from the
   * connection for display to the user.
   */
  function testConnectionConnect(&$button_form_values) {
    // Call the connect method.
    $connect = $this->callMethod('system.connect');

    if (is_array($connect) && isset($connect['user'])) {
      drupal_set_message(t('Sucessfully connected to the remote site.'));
    }
    else {
      drupal_set_message(t('Could not connect to the remote site.'), 'warning');
    }

    return $connect;
  }

  /**
   * Connection test button handler: user login.
   */
  function testConnectionLogin(&$button_form_values) {
    // Call the login method.
    $login = $this->callMethod('user.login');
    // Eep. we need user details!!!

    if (is_array($login) && isset($login['user'])) {
      drupal_set_message(t('Sucessfully logged in to the remote site; got back details for user %user (uid @uid).', array(
        '%user' => $login['user']['name'],
        '@uid'  => $login['user']['uid'],
        )));
    }
    else {
      drupal_set_message(t('Could not log in to the remote site.'), 'warning');
    }

    return $login;
  }

  /**
   * Connection test button validate handler: loading a node.
   */
  function testConnectionNodeLoadValidate(&$button_form_values) {
    if (empty($button_form_values['nid'])) {
      form_set_error('buttons][node_load][nid', 'Node id is required for the node retrieval test.');
    }
  }

  /**
   * Connection test button handler: loading a node.
   */
  function testConnectionNodeLoad(&$button_form_values) {
    // Must be cast to integer for faffiness of XMLRPC and Services.
    $nid = (int) $button_form_values['nid'];

    $fields = array();
    $node = $this->callMethod('node.get', $nid, $fields);

    if (is_array($node) && isset($node['nid'])) {
      drupal_set_message(t('Sucessfully retrieved node %title (nid @nid).', array(
        '%title' => $node['title'],
        '@nid'  => $node['nid'],
        )));
    }
    else {
      drupal_set_message(t('Could not retrieve a node from the remote site.'), 'warning');
    }

    return $node;
  }

}

/**
 * Drupal client for services on a Drupal 5 site.
 *
 * Works with Services 5.x-0.92.
 *
 * We extend from the Services 6.x-2.x class as not much actually changes
 * between these versions when it comes to making calls.
 */
class clients_connection_drupal_services_5 extends clients_connection_drupal_services_6_2 {
  /**
   * Call a remote method.
   *
   * TODO: REFACTOR this to look more like the static methods above --
   * separate methods for getuser, connect, etc etc.
   *
   * @param $method
   *  The name of the remote method to call.
   * @param
   *  All other parameters are passed to the remote method.
   *  Note that the D5 version of Services does not seem to respect optional parameters; you
   *  should pass in defaults (eg an empty string or 0) instead of omitting a parameter.
   *
   * @return
   *  Whatever is returned from the remote site.
   */
  function callMethod($method) {
    //dsm($this);
    //dsm($method);

    $config = $this->configuration;

    $connect = xmlrpc($this->endpoint, 'system.connect');
    $session_id = $connect['sessid'];

    // We may want to call only system.connect for testing purposes.
    if ($method == 'system.connect') {
      return $connect;
    }

    // Log in
    // Get the API key-related arguments.
    $key_args = $this->xmlrpc_key_args('user.login');
    //dsm($key_args);

    // Build the array of connection arguments we need to log in.
    $username = $this->configuration['username'];
    $password = $this->configuration['password'];
    $login_args = array_merge(
      array($this->endpoint, 'user.login'), // endpoint, method
      $key_args,                            // hash, domain_name, domain_time_stamp, nonce
      array($session_id),                   // sessid
      array($username, $password)           // username, password
    );

    // Call the xmlrpc method with our array of arguments. This accounts for
    // whether we use a key or not, and the extra parameters to pass to the method.
    $login = call_user_func_array('xmlrpc', $login_args);
    $login_session_id = $login['sessid'];

    // If the requested method is user.login, we're done.
    if ($method == 'user.login') {
      return $login;
    }

    //dsm($login);

    // Get all the arguments this function has been passed.
    $function_args  = func_get_args();
    // Slice out the ones that are arguments to the method call: everything past
    // the 1st argument.
    $method_args    = array_slice($function_args, 1);

    // The node.load method on D5 is an evil special case because it's defined
    // to not use an API key.
    if ($method == 'node.load') {
      // Be nice. Let the caller specify just the nid, and provide the
      // empty default for the optional fields parameter.
      if (count($method_args) == 1) {
        $method_args[] = array();
      }

      // Be nice part 2: the number one (in my experience) cause of lost
      // hours on Services is the way XMLRPC and/or services get their
      // knickers in a twist when they want an integer but think they've got
      // a string because they're too damn stupid to try casting.
      // So cast the nid here, since we're already in a special case for this
      // method anyway.
      $method_args[0] = (int) $method_args[0];

      // Build the array of connection arguments for the method we want to call.
      $xmlrpc_args = array_merge(
        array($this->endpoint, $method),
        array($login_session_id),
        $method_args
      );

      //dsm($xmlrpc_args);

      // Call the xmlrpc method with our array of arguments.
      $result = call_user_func_array('xmlrpc', $xmlrpc_args);

      if ($result === FALSE) {
        //dsm('error');
        return xmlrpc_error();
      }

      return $result;
    }

    // Get the API key-related arguments.
    $key_args = $this->xmlrpc_key_args($method);

    // Build the array of connection arguments for the method we want to call.
    $xmlrpc_args = array_merge(
      array($this->endpoint, $method),
      $key_args,
      array($login_session_id),
      $method_args
    );

    // Call the xmlrpc method with our array of arguments.
    $result = call_user_func_array('xmlrpc', $xmlrpc_args);

    if ($result === FALSE) {
      //dsm('error');
      return xmlrpc_error();
    }

    return $result;

  }

  /**
   * Provide buttons for the connection testing page.
   *
   * @param $form_state
   *  This is passed in so you can set defaults based on user input.
   */
  function getTestOperations($form_state, $cid) {
    // Just making the inheritance explicit for debugging... ;)
    return parent::getTestOperations($form_state, $cid);
  }

  /**
   * Connection test button handler: loading a node.
   *
   * This uses a different method on Services 5.x-0.92.
   */
  function testConnectionNodeLoad(&$button_form_values) {
    $nid = $button_form_values['nid'];

    $node = $this->callMethod('node.load', $nid);

    if (is_array($node) && isset($node['nid'])) {
      drupal_set_message(t('Sucessfully retrieved node %title (nid @nid).', array(
        '%title' => $node['title'],
        '@nid'  => $node['nid'],
        )));
    }
    else {
      drupal_set_message(t('Could not retrieve a node from the remote site.'), 'warning');
    }

    return $node;
  }


}

