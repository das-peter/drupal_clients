<?php
/**
 *
 */


/**
 *
 */
class ClientsServicesDrupal extends ClientsServicesBase {
    
	/**
	 * Use for testing
	 */
	static public function connect($serviceConfig)
	{
        $session = xmlrpc($serviceConfig->baseurl, 'system.connect');
        if ($session === FALSE) {
          return xmlrpc_error(); // does this work, seems to return NULL?
        }
        return $session;
    }

  /**
	 * Prepares a hashed token for the service, based on current time, the required service
	 * and config values; serviceKey and serviceDomain
   * @param stdClass $serviceConfig A service configuration as returned by Clients::load()
   * @param string $serviceMethod Name of service method to access
	 * 
	 * @return array a valid token
	 */
	static public function getToken($serviceConfig, $serviceMethod)
	{
    $timestamp = (string)time();
		$nonce = uniqid();
		$hashParameters = array($timestamp, $serviceConfig->domain, $nonce, $serviceMethod);
		$hash = hash_hmac("sha256", implode(';', $hashParameters), $serviceConfig->servicekey);

		return array(
            'hash' => $hash,
            'domain' => $serviceConfig->domain,
            'timestamp' => $timestamp,
            'nonce' => $nonce
        );
	}
    
  /**
	 * Connects to Drupal Services and logs in the user provided in the config.
	 * Returns a session for the user.
	 * 
	 * @return array
   * @todo needs error catching in case service is down 
	 */
	static public function getUser($serviceConfig)
	{
        $session = self::connect($serviceConfig);
        $userToken = self::getToken($serviceConfig, 'user.login');
        
        $user = xmlrpc(
            $serviceConfig->baseurl,
            'user.login',
            $userToken['hash'],
            $userToken['domain'],
            $userToken['timestamp'],
            $userToken['nonce'],
            $session['sessid'],
            $serviceConfig->username,
            $serviceConfig->password
         );
        if ($user === FALSE) {
          return xmlrpc_error();
        }

		return $user;
	}
    
    /**
     *
     */
    static public function call($serviceConfig, $serviceOptions) {
        if($serviceOptions['clients_methods'] == 'views.get') {

            $cacheid = md5($serviceConfig->name . implode($serviceOptions));
            $user = self::getUser($serviceConfig);

            // user is stdClass if xmlrpc_error()...
            if(!is_array($user) || !isset($user['sessid'])) {
                // TODO watchdog
                drupal_set_message($user->message, 'error');
                return;
            }
    
            $token = self::getToken($serviceConfig, $serviceOptions['clients_methods']);
            
            $result = parent::doCall(
                'xmlrpc',
                $cacheid,
                $serviceConfig->baseurl,
                $serviceOptions['clients_methods'], 
                $token['hash'],
                $token['domain'],
                $token['timestamp'],
                $token['nonce'],
                $user['sessid'],
                $serviceOptions['clients_options']['view'], 
                array(), // get all fields
                array_values($serviceOptions['clients_options']['arguments']), 
                (int)$serviceOptions['clients_options']['offset'], 
                (int)$serviceOptions['clients_options']['limit']
              );
            
            // needs some post-processing
            
            $processed_result = new stdClass;
            $processed_result->created = $result->created;
            $processed_result->data = array();
            foreach($result->data as $item) {
                // nid will interfere with local nids in client
                $item['remote_nid'] = $item['nid']; 
                unset($item['nid']);
                $tags = array();
                // remote taxonomy is not understood locally so flatten to RSS-style bag tags (TODO: develop this to preserve vocabs)
                foreach($item['taxonomy'] as $term) {
                    $tags = $term['name'];
                }
                $item['tags'] = $tags;
                unset($item['taxonomy']);
                
                // remove this section from module
                if($item['type'] == 'image') { // need to check if remote server has image service
                    $token2 = self::getToken($serviceConfig, 'image.getImages');
                    $images_result = parent::doCall(
                        'xmlrpc',
                        md5($token2['hash']), // edit
                        $serviceConfig->baseurl,
                        'image.getImages', 
                        $token2['hash'],
                        $token2['domain'],
                        $token2['timestamp'],
                        $token2['nonce'],
                        $user['sessid'],
                        (int)$item['remote_nid']
                      );
                    $item['images_data'] = $images_result->data;
                    //var_dump(xmlrpc_error()); var_dump($images_result); //die('images');
                }
                
                $processed_result->data[] = $item;
            }
            return $processed_result;
            
        } // else method not supported yet
    }
    
    // Following two functions are done in config now - need to remove some/all of this code
    
    /**
     * @return Array List of service methods
     */
    static public function getServiceMethods() {
        return array(
            'views.get',
        );
    }

    /**
     * @param String $method Name of method to get parameters for
     * @return Array FAPI form array of parameters for method
     */
    static public function getServiceParameters($method) {
        $form = array();
        if($method == 'views.get') {
            
            
            $views = array(
                'name_of_remote_view' => 'name_of_remote_view', 
            );
            $form['view'] = array(
                '#type' => 'select',
                '#title' => t('View'),
                '#default_value' => '',
                '#options' => $views,
                '#description' => t('Choose view'),
            );
            $form['arguments']['first'] = array(
                '#type' => 'textfield', 
                '#title' => t('Argument'), 
                '#default_value' => '', 
                '#size' => 10, 
                '#maxlength' => 30, 
            );
        }
        $form['offset'] = array(
            '#type' => 'textfield', 
            '#title' => t('Offset'), 
            '#default_value' => 0, 
            '#size' => 3, 
            '#maxlength' => 4, 
        );
        $form['limit'] = array(
            '#type' => 'textfield', 
            '#title' => t('Limit'), 
            '#default_value' => 0, 
            '#size' => 3, 
            '#maxlength' => 4, 
        );
        return $form;
    } 

    /**
     * @param mixed $service_identifier Pass in sid or name (both are unique)
     */
    static public function configLoad($serviceIdentifier) {
        if(is_int($serviceIdentifier)) {
            $result = db_query("SELECT * FROM {clients_services}, {clients_drupal} WHERE {clients_services}.sid = %d AND {clients_services}.sid = {clients_drupal}.sid", $serviceIdentifier);
        } elseif(is_string($serviceIdentifier)) {
            
            $result = db_query("SELECT * FROM {clients_services}, {clients_drupal} WHERE {clients_services}.name = '%s' AND {clients_services}.sid = {clients_drupal}.sid", $serviceIdentifier);
            
        } else {
            // TODO watchdog
            drupal_set_message('error');
            return; // or FALSE?
        }
        $config = db_fetch_object($result);
        $config->password = self::decrypt($config->password);
        return $config; 
    }

    /**
     * wrapping drupal_write_record for consistent API interface
     * @param Array $values Pass $form_state['values'] to this
     */
    static public function configSave($values) {
        
        $values['type'] = variable_get('services_drupal_type', 'services_drupal');
        $values['password'] = self::encrypt($values['password']);
        if(
            $resultbase = drupal_write_record('clients_services', $values) && 
            $resultdrupal = drupal_write_record('clients_drupal', $values)
           ) {
            // TODO add watchdog
            drupal_set_message('Added service');
        } else {
            drupal_set_message('Problem adding service - drupal_write_record() result: '. $resultbase .' - '. $resultdrupal);
        }
        return;
    }

    /**
     * wrapping drupal_write_record for consistent API interface
     * @param Array $values Pass $form_state['values'] to this
     */
    static public function configEdit($values) {
        
        $values['type'] = variable_get('services_drupal_type', 'services_drupal');
        
        if(empty($values['password'])) {
            unset($values['password']);
        }
        else {
            $values['password'] = self::encrypt($values['password']);
        }

        if(
            $resultbase = drupal_write_record('clients_services', $values, 'sid') && 
            $resultdrupal = drupal_write_record('clients_drupal', $values, 'sid')
           ) {
            // TODO add watchdog
            drupal_set_message('Edited service');
        } else {
            drupal_set_message('Problem editing service - drupal_write_record() result: '. $resultbase .' - '. $resultdrupal);
        }
        return;
    }

    /**
     *
     */
    static public function configDelete($sid) {
        if($sid) {
            $result =  db_query("DELETE FROM {clients_services} WHERE sid = %d LIMIT 1", $sid);
            $result =  db_query("DELETE FROM {clients_drupal} WHERE sid = %d LIMIT 1", $sid);
        }
    }
    
    /**
     *
     */
    static public function decrypt($encrypted) {
      $bf = self::blowfish();
      return trim($bf->decrypt($encrypted)); // that was fun to debug
    }
    
    /**
     *
     */
    static public function encrypt($plaintext) {
      $bf = self::blowfish();
      return $bf->encrypt($plaintext);
    }
    
    /**
     *
     */
    static protected function blowfish() {
      ini_set('include_path',ini_get('include_path') .':'. variable_get('services_drupal_cryptdir', ''));
      include_once('Crypt/Blowfish.php');
      return new Crypt_Blowfish(self::getkey()); 
    }
    
    /**
     *
     */
    static public function getkey() {
      $filename = drupal_get_path('module', 'services_drupal') .'/services_drupal_key.inc';
      $handle = fopen($filename, "r");
      $key = fread($handle, filesize($filename));
      fclose($handle);    
      return $key;
    }
    
    
}