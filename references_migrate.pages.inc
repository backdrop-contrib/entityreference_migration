<?php

function references_migrate_migrate_references_fields() {
  $form = array();

  // Load all references fields.
  $node_field_infos = field_read_fields(
    array('type'=>'node_reference'),
    array('include_inactive' => TRUE, 'include_deleted' => TRUE)
  );
  $user_field_infos = field_read_fields(
    array('type'=>'user_reference'),
    array('include_inactive' => TRUE, 'include_deleted' => TRUE)
  );

  $field_infos = $node_field_infos + $user_field_infos;
  
  $field_options = array();
  
  foreach ($field_infos as $key => $field_info) {
    $translate_array = array('@field_name' => $field_info['field_name']);
    $instances = field_read_instances(
      array('field_name' => $field_info['field_name']),
      array('include_inactive' => TRUE, 'include_deleted' => TRUE)
    );
    dpm($instances);
    $options_instances = array();
    foreach ($instances as $instance) {
      $options_instances[] = $instance['entity_type'] .':'. $instance['bundle'] .' ('. check_plain($instance['label']) .')';
    }
    $translate_array['@instances'] = implode(', ', $options_instances);
    $field_options[$key] = t('@field_name: Appears in [@instances]', $translate_array);
  }
  
  $form['#field_info'] = array(
    '#type' => 'value',
    '#value' => $field_infos,
  );
  
  $form['field_options'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Which references fields do you wish to migrate?'),
    '#options' => $field_options,
  );

  dpm($field_infos);
  return confirm_form($form, t('Are you sure you wish to migrate the selected references fields?'), '<front>');;
}