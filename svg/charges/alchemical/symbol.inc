<?php
/**
 * Drawshield PHP Source File
 *
 * Description...
 *
 * @author Karl R. Wilcox <karl@karlwilcox.com>
 * @copyright Karl R. Wilcox 2014
 * @license http://www.apache.org/licenses/LICENSE-2.0
 * Date: 11/09/14
 * Time: 23:00
 */


$item = strtolower(getModifierValueByKeyterm($node,'value'));
if ( $item == 'sulfur' ) {
  $charge['file'] = 'sulphur.svg';
} elseif ( file_exists( $folder . $item . '.svg')) {
  $charge['file'] = "$item.svg";
} else {
  $charge['file'] = "fire.svg";
  $messages->addMessage('warning',"No symbol for $item (using fire)");
}

