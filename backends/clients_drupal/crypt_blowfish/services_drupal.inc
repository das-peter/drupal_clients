<?php
// $Id$
/**
 *
 * @author Django Beatty - adub
 */


/**
 *
 */
class ClientsServicesDrupal extends ClientsServicesBase {
    
	/**
	 * Use for testing
	 */
	static public function connect($connection)
	{
        $session = xmlrpc($connection->baseurl, 'system.connect');
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
	static public function getToken($connection, $serviceMethod)
	{
    $timestamp = (string)time();
		$nonce = uniqid();
		$hashParameters	= array($timestamp, $connection->configuration['domain'], $nonce, $serviceMethod);
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
	 * 
	 * @return array
     * @todo needs error catching in case service is down 
	 */
	static public function getUser($connection)
	{
        $session = self::connect($connection);
        $userToken = self::getToken($connection, 'user.login');
        
        $user = xmlrpc(
            $connection->baseurl,
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
     *
     */
    static public function call($serviceConfig, $serviceOptions) {
        if($serviceOptions->configuration['options']['method'] == 'views.get') {

            $cacheid = md5($serviceConfig->name . implode($serviceOptions->configuration['options']));
            
            $user = self::getUser($serviceConfig);
            
            // user is stdClass if xmlrpc_error()...
            if(!is_array($user) || !isset($user['sessid'])) {
                // TODO watchdog
                drupal_set_message($user->message, 'error');
                return;
            }
    
            $token = self::getToken($serviceConfig, $serviceOptions->configuration['options']['method']); 
            
            $result = parent::doCall(
                'xmlrpc',
                $cacheid,
                $serviceConfig->baseurl,
                $serviceOptions->configuration['options']['method'], 
                $token['hash'],
                $token['domain'],
                $token['timestamp'],
                $token['nonce'],
                $user['sessid'],
                $serviceOptions->configuration['options']['view'], 
                array(), // get all fields
                array_values($serviceOptions->configuration['options']['arguments']), 
                (int)$serviceOptions->configuration['options']['offset'], 
                (int)$serviceOptions->configuration['options']['limit']
              );
            
            // needs some post-processing
//            dsm('raw result'); dvm($result);
            
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
    

    /**
     * @param mixed $service_identifier Pass in sid or name (both are unique)
     */
    static public function configLoad($serviceIdentifier) {
        if(is_int($serviceIdentifier)) {
            $result = db_query("SELECT * FROM {clients_connections} WHERE {clients_connections}.sid = %d", $serviceIdentifier);
        } elseif(is_string($serviceIdentifier)) {
            $result = db_query("SELECT * FROM {clients_connections} WHERE {clients_connections}.name = '%s'", $serviceIdentifier);
        } else {
            // TODO watchdog
            drupal_set_message('error');
            return; // or FALSE?
        }
        $connection = db_fetch_object($result);

        $connection->configuration = unserialize($connection->configuration);
        $connection->configuration['password'] = self::decrypt(base64_decode($connection->configuration['password']));
        return $connection; 
    }

    /**
     * wrapping drupal_write_record for consistent API interface
     * @param Array $values Pass $form_state['values'] to this
     */
    static public function configSave($values) {
 //       var_dump($values); die(); // not hitting this????
        $values['type'] = variable_get('services_drupal_type', 'services_drupal');
        $values['configuration']['password'] = base64_encode(self::encrypt($values['configuration']['password']));
//dpm($values);
        if(
            $resultbase = drupal_write_record('clients_connections', $values)
           ) {
            // TODO add watchdog
            drupal_set_message('Added connection');
        } else {
            drupal_set_message('Problem adding connection - drupal_write_record() result: '. $resultbase);
        }
        return;
    }

    /**
     * wrapping drupal_write_record for consistent API interface
     * @param Array $values Pass $form_state['values'] to this
     */
    static public function configEdit($values) {
        $values['type'] = variable_get('services_drupal_type', 'services_drupal');
        
        // prepare pass for serialized storage
        if(empty($values['configuration']['password'])) {
          // need to load connection and set password to original if blank
          $original = self::configLoad((int)$values['sid']);
          $values['configuration']['password'] = $original->configuration['password'];
        }
        $values['configuration']['password'] = base64_encode(self::encrypt($values['configuration']['password']));
        
        if(
            $resultbase = drupal_write_record('clients_connections', $values, 'sid')
           ) {
            // TODO add watchdog
            drupal_set_message('Edited connection');
        } else {
            drupal_set_message('Problem editing connection - drupal_write_record() result: '. $resultbase);
        }
        return;
    }

    /**
     *
     */
    static public function configDelete($sid) {
        if($sid) {
            $result =  db_query("DELETE FROM {clients_connections} WHERE sid = %d LIMIT 1", $sid);
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