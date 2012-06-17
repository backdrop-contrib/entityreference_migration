<?php

/**
 * @file
 * PHP script to convert References fields to Entitityreference fields.
 * 
 * Execute from the command line, using Drush:
 *  drush scr references-migrate.php
 */

$node_field_infos = field_read_fields(
  array('type'=>'node_reference'),
  array('include_inactive' => TRUE, 'include_deleted' => TRUE)
);
$user_field_infos = field_read_fields(
  array('type'=>'user_reference'),
  array('include_inactive' => TRUE, 'include_deleted' => TRUE)
);
$field_infos = $node_field_infos + $user_field_infos;
foreach ($field_infos as $key => $field_info) {
  $field_name = $field_info['field_name'];

  // Create table for migration.
  $table_name = _field_sql_storage_tablename($field_info);
  $temp_table_name = 'entityreference_migration_' . $table_name;
  if (!db_table_exists($temp_table_name)) {
    $schema = drupal_get_schema($table_name, TRUE);
    $schema['name'] = $temp_table_name;
    db_create_table($temp_table_name, $schema);
  }

  // Make sure we don't stumble over leftovers of broken migrations.
  $deleted_table_name = _field_sql_storage_tablename(array('deleted' => TRUE) + $field_info);
  $deleted_revision_table_name = _field_sql_storage_revision_tablename(array('deleted' => TRUE) + $field_info);
  if ($key == $field_info['id'] && db_table_exists($table_name)) {
    if (db_table_exists($deleted_table_name)) {
      db_drop_table($deleted_table_name);
    }
    if (db_table_exists($deleted_revision_table_name)) {
      db_drop_table($deleted_revision_table_name);
    }
  }

  // Export data.
  if (db_table_exists($table_name)) {
    $results = db_select($table_name)
      ->fields($table_name)
      ->execute()
      ->fetchAll(PDO::FETCH_ASSOC);
    if ($results) {

      db_truncate($temp_table_name)->execute();

      $result = array_shift($results);
      $insert = db_insert($temp_table_name);
      $insert->fields(array_keys($result), $result);
      foreach ($results as $result) {
        $insert->values($result);
      }
      $insert->execute();
    }
  }

  // Store instances to be able to recreate them.
  $instances = field_read_instances(
    array('field_name' => $field_name),
    array('include_inactive' => TRUE, 'include_deleted' => TRUE)
  );

  foreach ($instances as $instance) {
    field_delete_instance($instance);
    field_purge_instance($instance);
  }
  field_delete_field($field_name);
  field_purge_field($field_info);

  // Recreate fields by using entityreference.
  $target_bundles = array();
  if (isset($field_info['settings']['referenceable_types'])) {
    $target_bundles = array_filter($field_info['settings']['referenceable_types']);
  }
  $entityreference_field = array(
    'field_name' => $field_name,
    'type' => 'entityreference',
    'module' => 'entityreference',
    'entity_types' => array(),
    'foreign keys' => array(),
    'indexes' => array(
      'target_entity' => array(
        '0' => 'target_type',
        '1' => 'target_id',
      ),
    ),
    'settings' => array(
      'handler' => 'base',
      'handler_settings' => array (
        'target_bundles' => $target_bundles,
      ),
      'handler_submit' => 'Change handler',
      'target_type' => (($field_info['type'] == 'node_reference') ? 'node' : 'user'),
    ),
    'cardinality' => $field_info['cardinality'],
    'locked' => $field_info['locked'],
    'active' => $field_info['active'],
    'translatable' => $field_info['translatable'],
  );
  $entityreference_field = field_create_field($entityreference_field);

  // Create the field instances
  foreach ($instances as $instance) {
    $entityrefrence_instance = array(
      'bundle' => $instance['bundle'],
      'default_value' => $instance['default_value'],
      'description' => $instance['description'],
      'display' => array(
        'default' => array(
          'label' => 'above',
          'module' => 'entityreference',
          'settings' => array(
            'link' => FALSE,
          ),
          'type' => 'entityreference_label',
        ),
      ),
      'entity_type' => $instance['entity_type'],
      'field_name' => $instance['field_name'],
      'label' => $instance['label'],
      'required' => $instance['required'],
    );
    $entityrefrence_instance = field_create_instance($entityrefrence_instance);
  }

  $entityreference_table_name = _field_sql_storage_tablename($entityreference_field);
  $entityreference_revision_table_name = _field_sql_storage_revision_tablename($entityreference_field);

  // And now migrate data.
  $results = db_select($temp_table_name)
    ->fields($temp_table_name)
    ->execute()
    ->fetchAll(PDO::FETCH_ASSOC);

  if ($results) {
    db_truncate($entityreference_table_name)->execute();
    db_truncate($entityreference_revision_table_name)->execute();

    $insert = db_insert($entityreference_table_name);
    $insert_revision = db_insert($entityreference_revision_table_name);
    foreach ($results as $key => $result) {
      $target_id_field = $field_name . (($field_info['type'] == 'node_reference') ? '_nid' : '_uid');
      $result[$field_name . '_target_id'] = $result[$target_id_field];
      $result[$field_name . '_target_type'] = $entityreference_field['settings']['target_type'];
      unset($result[$target_id_field]);

      if ($key) {
        $insert->values($result);
        $insert_revision->values($result);
      }
      else {
        $insert->fields(array_keys($result), $result);
        $insert_revision->fields(array_keys($result), $result);
      }
    }
    $insert->execute();
    $insert_revision->execute();
    db_drop_table($temp_table_name);
    // Cleanup
    if (db_table_exists($deleted_table_name)) {
      db_drop_table($deleted_table_name);
    }
    if (db_table_exists($deleted_revision_table_name)) {
      db_drop_table($deleted_revision_table_name);
    }
  }
}
