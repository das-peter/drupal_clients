<?php
/**
 * @file
 * Contains classes for Client connections handlers.
 */

/**
 * Base class for Drupal client connections.
 */
class clients_connection_drupal_services extends clients_connection_base {
  // ============================================ Base: Constructor.
  /**
   * Constructor method.
   *
   * @param $object
   *  An object of class stdClass returned from CTools.
   */
  function __construct($object) {
    // Call the base class to set the connection properties.
    parent::__construct($object);

    // Decrypt the password.
    $this->configuration['password'] = clients_drupal_decrypt($this->configuration['password']);
  }

  // ============================================ Base: Testing system.
  /**
   * Provide buttons for the connection testing page.
   *
   * @param $form_state
   *  This is passed in so you can set defaults based on user input.
   */
  function getTestOperations($form_state, $cid) {
    $buttons['connect'] = array(
      '#value' => t('Test connection'),
      '#type' => 'submit',
      //'#name' => 'connect', // wtf does this do?
      '#action_type' => 'method',
      '#action_submit' => 'testConnectionConnect',
      '#description' => t('Test the connection settings by calling system.connect on the remote server.'),
    );
    $buttons['login'] = array(
      '#value' => t('Test user login'),
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
      '#title' => t('Node ID'),
      '#size' => 6,
      '#default_value' => isset($form_state['values']['buttons']['node_load']['nid']) ? $form_state['values']['buttons']['node_load']['nid'] : NULL,
    );
    $buttons['node_load']['button'] = array(
      '#value' => t('Test node retrieval'),
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
    $connect = $this->callMethodArray('system.connect');

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
    $login = $this->callMethodArray('user.login');
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
      form_set_error('buttons][node_load][nid', t('Node id is required for the node retrieval test.'));
    }
  }

  /**
   * Connection test button handler: loading a node.
   *
   * Each version has to do its own thing; this is just here to avoid a WTF.
   */
  function testConnectionNodeLoad(&$button_form_values) {
  }

}

/**
 * Drupal client for services on a Drupal 7 site for Services 7.x-3.x.
 */
class clients_connection_drupal_services_7_3 extends clients_connection_drupal_services {
  // ============================================ D7-3: Connection form methods.
  /**
   * Extra form elements specific to a class's edit form.
   *
   * This is the same pattern as node_form() -- just ignore the object behind
   * the curtain ;)
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

    $form['endpoint'] = array(
      '#type' => 'textfield',
      '#title' => t('Connection endpoint'),
      '#default_value' => $this->new ? '' : $this->endpoint,
      '#size' => 50,
      '#maxlength' => 100,
      '#description' => t('Remote service URL e.g. http://mysite.com/path/to/xmlrpc'),
      '#required' => TRUE,
    );

    $form['configuration'] = array(
      '#type' => 'fieldset',
      '#title' => t('Configuration'),
      '#collapsible' => FALSE,
      '#tree' => TRUE,
    );

    $form['configuration']['username'] = array(
      '#type' => 'textfield',
      '#title' => t('Service username'),
      '#default_value' => $this->new ? '' : $this->configuration['username'],
      '#size' => 30,
      '#maxlength' => 60,
      '#attributes' => array('autocomplete' => 'off'),
      '#description' => t('This should be same as the username on the server you are connecting to.'),
      '#required' => TRUE,
    );

    $password_desc = $this->new
      ? t('This should be same as the password on the server you are connecting to.')
      : t('This should be same as the password on the server you are connecting to. Leave blank unless you need to change this.');
    $form['configuration']['password'] = array(
      '#type' => 'password',
      '#title' => t('Service password'),
      '#size' => 30,
      '#maxlength' => 60,
      '#attributes' => array('autocomplete' => 'off'),
      '#description' => $password_desc,
      '#required' => $this->new,
    );

    return $form;
  }

  /**
   * Submit handler for saving/updating connections of this class.
   *
   * @see clients_connection_form_submit()
   */
  static function connectionSettingsForm_submit($form, &$form_state) {
    $old_connection = $form_state['values']['old_connection'];

    // Check whether we're editing or adding a new connection.
    if ($old_connection->new) {
      $form_state['values']['configuration']['password'] = clients_drupal_encrypt($form_state['values']['configuration']['password']);
    }
    else {
      // Prepare password for serialized storage
      if (empty($form_state['values']['configuration']['password'])) {
        // Set password to original if blank.
        $form_state['values']['configuration']['password'] = $old_connection->configuration['password'];
      }
      $form_state['values']['configuration']['password'] = clients_drupal_encrypt($form_state['values']['configuration']['password']);
    }
  }

  function formatEndpoint($url) {
    return $url;
  }

  // ============================================ D7-3: Connection API.
  /**
   * Call a remote method.
   *
   * @param $method
   *  The name of the remote method to call.
   * @param $method_params
   *  An array of parameters to passed to the remote method.
   *  Note that the D5 version of Services does not seem to respect optional parameters; you
   *  should pass in defaults (eg an empty string or 0) instead of omitting a parameter.
   *
   * @return
   *  Whatever is returned from the remote site.
   */
  function callMethodArray($method, $method_params = array()) {
    // Connect to the remote system service to get an initial session id to log in with.
    $connect = xmlrpc($this->endpoint, 'system.connect');
    $session_id = $connect['sessid'];

    // We may want to call only system.connect for testing purposes.
    if ($method == 'system.connect') {
      return $connect;
    }

    // Log in and get the user's session ID.
    $username = $this->configuration['username'];
    $password = $this->configuration['password'];

    $login = xmlrpc($this->endpoint, 'user.login', $username, $password);
    $login_session_id = $login['sessid'];

    // If the requested method is user.login, we're done.
    if ($method == 'user.login') {
      return $login;
    }

    // Build the array of connection arguments for the method we want to call.
    $xmlrpc_args = array_merge(
      array($this->endpoint, $method),
      $method_params
    );

    // Call the xmlrpc method with our array of arguments.
    $result = call_user_func_array('xmlrpc', $xmlrpc_args);

    if ($result === FALSE) {
      //dsm('error');
      return xmlrpc_error();
    }

    return $result;
  }

  // ============================================ D7-3: Testing system.
  /**
   * Connection test button handler: loading a node: Services 7.x-3.x.
   */
  function testConnectionNodeLoad(&$button_form_values) {
    // Must be cast to integer for faffiness of XMLRPC and Services.
    $nid = (int) $button_form_values['nid'];

    $node = $this->callMethodArray('node.retrieve', array($nid));

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
 * Drupal client for services on a Drupal 6 site for Services 6.x-2.x.
 *
 * Developed against Services 6.x-2.4.
 *
 * Note that the Services 5.x-0.92 class is a subclass of this and uses several
 * of its methods.
 */
class clients_connection_drupal_services_6_2 extends clients_connection_drupal_services {
  // ============================================ D6-2: Connection form methods.
  /**
   * Extra form elements specific to a class's edit form.
   *
   * This is the same pattern as node_form() -- just ignore the object behind
   * the curtain ;)
   *
   * This is common to versions 6-2 and 5 of Drupal Services.
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

    $form['endpoint'] = array(
      '#type' => 'textfield',
      '#title' => t('Connection endpoint'),
      '#default_value' => $this->new ? '' : $this->endpoint,
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
      '#default_value' => $this->new ? '' : $this->configuration['domain'],
      '#size' => 50,
      '#maxlength' => 100,
      '#description' => t('This should be same as the \'Domain\' field used by the Services authentication key on the server you are connecting to.'),
      '#required' => TRUE,
    );
    $form['configuration']['servicekey'] = array(
      '#type' => 'textfield',
      '#title' => t('Service key'),
      '#default_value' => $this->new ? '' : $this->configuration['servicekey'],
      '#size' => 50,
      '#maxlength' => 40,
      '#attributes' => array('autocomplete' => 'off'),
      '#description' => t('This should be same as the \'Key\' field used by the Services authentication key on the server you are connecting to.'),
      '#required' => TRUE,
    );
    $form['configuration']['username'] = array(
      '#type' => 'textfield',
      '#title' => t('Service username'),
      '#default_value' => $this->new ? '' : $this->configuration['username'],
      '#size' => 30,
      '#maxlength' => 60,
      '#attributes' => array('autocomplete' => 'off'),
      '#description' => t('This should be same as the username on the server you are connecting to.'),
      '#required' => TRUE,
    );

    $password_desc = $this->new
      ? t('This should be same as the password on the server you are connecting to.')
      : t('This should be same as the password on the server you are connecting to. Leave blank unless you need to change this.');
    $form['configuration']['password'] = array(
      '#type' => 'password',
      '#title' => t('Service password'),
      '#size' => 30,
      '#maxlength' => 60,
      '#attributes' => array('autocomplete' => 'off'),
      '#description' => $password_desc,
      '#required' => $this->new,
    );

    return $form;
  }

  /**
   * Submit handler for saving/updating connections of this class.
   *
   * @see clients_connection_form_submit()
   */
  static function connectionSettingsForm_submit($form, &$form_state) {
    $old_connection = $form_state['values']['old_connection'];

    // Check whether we're editing or adding a new connection.
    if ($old_connection->new) {
      $form_state['values']['configuration']['password'] = clients_drupal_encrypt($form_state['values']['configuration']['password']);
    }
    else {
      // Prepare password for serialized storage
      if (empty($form_state['values']['configuration']['password'])) {
        // Set password to original if blank.
        $form_state['values']['configuration']['password'] = $old_connection->configuration['password'];
      }
      $form_state['values']['configuration']['password'] = clients_drupal_encrypt($form_state['values']['configuration']['password']);
    }
  }

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
    $base_url = str_replace('services/xmlrpc', '', $url);
    $link = l($base_url, $base_url);
    return $link . 'services/xmlrpc';
  }

  // ============================================ D6-2: Connection methods.
  /**
   * Log in as the configured user.
   *
   * Helper for callMethodArray(), because logging in doesn't change between
   * versions of Services.
   *
   * @param $session_id
   *  A session ID obtained from calling system.connect.
   *
   * @return
   *  The full data returned from the remote call.
   */
  function call_user_login($session_id) {
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

    return $login;
  }

  /**
   * Call a remote method with an array of parameters.
   *
   * This is technically internal; use the more DX-friendly callMethod() or
   * the all-in-one clients_connection_call().
   *
   * @param $method
   *  The name of the remote method to call.
   * @param
   *  All other parameters are passed to the remote method.
   *
   * @return
   *  Whatever is returned from the remote site.
   */
  function callMethodArray($method, $method_params = array()) {
    // If HTTP requests are enabled, report the error and do nothing.
    // (Cribbed from Content distribution module.)
    if (variable_get('drupal_http_request_fails', FALSE) == TRUE) {
      drupal_set_message(t('Drupal is unable to make HTTP requests. Please reset the HTTP request status.'), 'error', FALSE);
      watchdog('clients', 'Drupal is unable to make HTTP requests. Please reset the HTTP request status.', array(), WATCHDOG_CRITICAL);
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

    // Log in and get the user's session ID.
    $login = $this->call_user_login($session_id);
    $login_session_id = $login['sessid'];

    // If the requested method is user.login, we're done.
    if ($method == 'user.login') {
      return $login;
    }

    // Get the API key-related arguments.
    $key_args = $this->xmlrpc_key_args($method);

    // Build the array of connection arguments for the method we want to call.
    $xmlrpc_args = array_merge(
      array($this->endpoint, $method),
      $key_args,
      array($login_session_id),
      $method_params
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
   */
  function xmlrpc_key_args($method) {
    $api_key = $this->configuration['servicekey'];

    // Build the API key arguments - if no key supplied supplied, presume not required
    if ($api_key != '') {
      // Use API key to get a hash code for the service.
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

  // ============================================ D6-2: Testing system.
  /**
   * Connection test button handler: loading a node on D6.
   */
  function testConnectionNodeLoad(&$button_form_values) {
    // Must be cast to integer for faffiness of XMLRPC and Services.
    $nid = (int) $button_form_values['nid'];

    $fields = array();
    $node = $this->callMethodArray('node.get', array($nid, $fields));

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
  // ============================================ D5: Connection methods.  
  /**
   * Call a remote method with an array of parameters.
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
  function callMethodArray($method, $method_params = array()) {
    //dsm($this);
    //dsm($method);

    $config = $this->configuration;

    $connect = xmlrpc($this->endpoint, 'system.connect');
    $session_id = $connect['sessid'];

    // We may want to call only system.connect for testing purposes.
    if ($method == 'system.connect') {
      return $connect;
    }

    // Log in and get the user's session ID.
    $login = $this->call_user_login($session_id);
    $login_session_id = $login['sessid'];

    // If the requested method is user.login, we're done.
    if ($method == 'user.login') {
      return $login;
    }

    //dsm($login);

    // The node.load method on D5 is an evil special case because it's defined
    // to not use an API key.
    if ($method == 'node.load') {
      // Be nice. Let the caller specify just the nid, and provide the
      // empty default for the optional fields parameter.
      if (count($method_params) == 1) {
        $method_params[] = array();
      }

      // Be nice part 2: the number one (in my experience) cause of lost
      // hours on Services is the way XMLRPC and/or services get their
      // knickers in a twist when they want an integer but think they've got
      // a string because they're too damn stupid to try casting.
      // So cast the nid here, since we're already in a special case for this
      // method anyway.
      $method_params[0] = (int) $method_params[0];

      // Build the array of connection arguments for the method we want to call.
      $xmlrpc_args = array_merge(
        array($this->endpoint, $method),
        array($login_session_id),
        $method_params
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
      $method_params
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

  // ============================================ D5: Testing system.
  /**
   * Connection test button handler: loading a node.
   *
   * This uses a different method on Services 5.x-0.92.
   */
  function testConnectionNodeLoad(&$button_form_values) {
    $nid = $button_form_values['nid'];

    $node = $this->callMethodArray('node.load', array($nid));

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

