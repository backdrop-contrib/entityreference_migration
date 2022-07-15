# Entity Reference Migration

Migrate References fields (node and user) to Entity Reference fields.

## Installation

Install this module using the official Backdrop CMS instructions at <https://backdropcms.org/guide/modules>.

## Configuration

Ensure you have a full database and configuration backup before proceeding!

Navigate to `admin/content/migrate-references`, select the fields you wish to convert,
and click confirm.

## Drush

Type 'drush entityreference-migrate-references' (or 'drush emr') to convert all
fields. You may provide a field machine name as an argument to convert on a
field-by-field basis.

## Pre-conversion task list

Before starting any conversion, there are a few recommended tasks.

### Fields

* Create a where-used list of fields, widgets and formatters, using:
* core: /admin/reports/fields
* contrib: <https://drupal.org/project/field_info>

### Views

* Create a where-used list of fields, filter criteria, sort criteria, contextual
filters, using:
  * views: /admin/reports/fields/views-fields
  * There are some issues when you have a entityreference as an exposed filter,
see: <https://drupal.org/project/issues/entityreference?text=exposed+filter>

### Custom code

* Check your custom code that explicitly calls on data stored in references
format.

### Backup your data and configuration

This is (very) strongly recommended.  This is a one way conversion and data may
not be easily restored if something goes wrong. Having a backup will ensure you
have a safe point to revert your site.

Make a backup of your database.  You'll also need to make a copy of your
site configuration. One option is to install Backup and Migrate.

## Post-conversion task list

Test all CRUD operation for each entity.

### Fields

For each field:

* restore the widget: it is reset tot Autocomplete by default;
* restore the formatter of each View mode; it is set to "Label, with link to
referenced entity" by default

### Views

* Check any views where you used the entityreference; they may have broken
handlers and will need to be rebuilt.
* For each mentioned View, check each display and test thoroughly!
* If you have dev environment and/or use features, perform the changes locally
and check your views. You will have the ability to export them and import into
your live site after the conversion. Using features would be even easier as you
can just revert to your new views that utilize the entity reference handler.

### Custom code

Again, check your custom code.

## Issues

To submit bug reports and feature suggestions, or to track changes:
  <https://github.com/backdrop-contrib/entityreference_migration/issues>

## Current Maintainers

* [Herb v/d Dool](https://github.com/herbdool/)
* Seeking co-maintainers.

## Credits

* Ported to Backdrop by [Herb v/d Dool](https://github.com/herbdool/).
* Originally developed for [Drupal](https://www.drupal.org/project/entityreference_migration) by [BTMash](https://www.drupal.org/u/btmash).

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.
