<?php

/**
 * @file
 * Provides classes for clients handler entities.
 */

/**
 * Base class for entities that function as handlers.
 *
 * Part of this could be abstracted -- see the controller class.
 */
class ClientsHandlerEntity extends Entity {

  /**
   * Constructor method.
   *
   * Overrides Entity::__construct().
   */
  function __construct(array $values = array(), $entityType = NULL) {
    // todo: refactor; use parent::()?
    $this->entityType = $entityType;
    $this->setUp();

    // Lump all data unto the object...
    foreach ($values as $field => $value) {
      $this->$field = $value;
    }

    // Connections defined in code are already unserialized.
    // TODO: check if still needed???
    if (isset($object->configuration) && !is_array($object->configuration)) {
      $this->configuration = unserialize($object->configuration);
    }

    return $this;
  }

}

/**
 * Class for broken handlers.
 */
class clients_resource_broken extends ClientsHandlerEntity {
  // TODOL clients_resource_broken
  // $data->broken_message = t('The class for this connection is missing. Perhaps the module providing this is not available?');

}
