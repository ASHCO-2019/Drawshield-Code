<?php
	 

function createBody($spec, $lineType, $feature_size) {
  $spec = subPercent($spec);
  return '<path fill-rule="evenodd" d="' . makePath ( $spec, $lineType, $feature_size ) . '" />';
} 

function makeVoid ( $spec, $lineType = 'none', $featureSize = 50, $offsetSize = 30 ) {
  $spec = subPercent($spec);
  $retval = '';
  // Create the outer edge as normal
  $retval .= makePath ( $spec, $lineType, $featureSize );
  $innerSpec = parallelSpec ( $spec, 'i', $offsetSize );
  $retval .= ' ' . makePath ( $innerSpec, 'none', 0 );
  return $retval;
}

function createVoid($spec, $lineType, $feature_size) {
  $spec = subPercent($spec);
  $retval = '<path fill-rule="evenodd" d="' . makeVoid ( $spec, $lineType, $feature_size ) . '" />';
  return $retval;
}

function createCotise($spec, $num, $lineType) {
  $spec = subPercent($spec);
  $cotiseSize = 40;
  $outerSize = $cotiseSize*($num*2);
  $innerSize = $cotiseSize*(($num*2)-1);
  $outerSpec = parallelSpec ( $spec, 'o', $outerSize );
  $innerSpec = parallelSpec ( $spec, 'o', $innerSize );
  $innerSpec = reverseSpec ( $innerSpec );
  $retval = '<path fill-rule="evenodd" d="';
  $retval .= makePath ( $outerSpec, $lineType, 18 );
  $retval .= ' ' . makePath ( $innerSpec, $lineType, 18 );
  $retval .= '" />';
  return $retval;
}

function division_to_modifier(DOMElement &$node, $type) {
	// check for tincture being a quarterly division, if so handle separately & rewrite modifiers
  /**
   * @var DOMElement $item
   */
  $item = null;
	// See if there is a division of the required type
  if ($node != null) {
  	foreach ( $node->childNodes as $child ) {
  		if ( $child->nodeName == 'tincture' ) {
        $firstChild = resolveReference($child)->firstChild;
        if ( $firstChild != null && $firstChild->nodeName == 'division' && $firstChild->getAttribute('keyterm') == $type ) {
    		  $item = $child;
    			break;
        }
  		}
  	}
  }
	if ( $item == null ) return false; // not found

  /**
   * @var DOMElement[] $children
   */
  $children = array();
	// We cannot just iterate through childNodes as we want to make changes, so put in an array
	foreach ( $item->firstChild->childNodes as $child ) {
	  $children[] = (object)$child;
	}
	// We can now remove the original tincture
	$node->removeChild($item);
	// Create a new modifier node
	$mod = createModifierNode('div2ord',$type);
	// Now copy any tinctures back into the original node, and save any other modifiers
	for ( $i = 0; $i < count($children); $i++ )
	  if ( $children[$i]->nodeName == 'tincture' ) {
			// We swap the colours over, as body2 draws the first one (other way round for normal divisions)
			if ( $children[$i]->getAttribute('index') == '1')			
			  $children[$i]->setAttribute('index','2');
			else
			  $children[$i]->setAttribute('index','1');
		  $node->appendChild($children[$i]);
		}
		else
		  $mod->appendChild($children[$i]);
	// Create a modifier to show which division we replaced
	$node->appendChild($mod);

  return true;
}


function includeOrdinary( DOMElement &$node ) {
  global $messages, $options;

  $ordinaryType = $node->getAttribute('keyterm');
  $ordinary = array ();
  $file = null;
  if ($options['shape'] == 'flag') { // look in the flag sub-directory first
    $file ="ordinaries/flag/$ordinaryType.inc";
    if ( stream_resolve_include_path($file) === false ) $file = null;
  }
  if ($file == null) {
    $file = "ordinaries/$ordinaryType.inc";
    if ( stream_resolve_include_path($file) === false ) $file = null;
  }
  $calcHeight = 1200; // value to use for calculating drawing height
  if ($options['shape'] == 'flag') {
    $calcHeight = $options['flagHeight'];
  }
  if ($file != null) { // calcHeight is available within the included file
    include ( $file );
  } else { // can't draw this ordinary
    $messages->addMessage('warning',"Cannot draw $ordinaryType, substituting bend" . getLine($node) );
    include ( 'ordinaries/bend.inc');
  }
  return $ordinary;
}

function getSpecial($type,$node) {
    global $options, $messages;
    $special = [];

    $filename = $options['shape'];
    if ($filename == 'swatch' || $filename == 'stamp') $filename = "quarter"; // basically the same shape
    if ($filename == 'flag') {
        $height = (isset($options['flagHeight'])) ? $options['flagHeight'] : 800; // just in case
        $size = (int)(min(100,$height/8));
        $special['body'] = '<path fill-rule="evenodd" d="M -10,-10 h1020 v' .
            ($height + 20) . ' h-1020 v-' . ($height + 20) . ' z M' . $size . ', ' . $size .
            ' h ' . (1000 - $size - $size) . ' v ' . ($height - $size - $size) . ' h-' .
            (1000 - $size - $size) . ' z"/>';
        return ($special);
    } // else
    if (chiefPresent()) $filename .= '-chief';
    $linetype = getModifierByType($node, 'linetype', false);
    if ($linetype != null) $filename .= '-' . $linetype;
    $compony = getModifierByType($node,'compony');
    if ($compony != null) $filename .= '-' . $compony;
// $filename .= '.svg';
    $folder = "svg/ordinaries/$type/";

    $subfile = getBestMatchFilename($folder, $filename, null);

    if ($subfile == null) {
        $messages->addMessage('warning',"Cannot draw $type inside " . $options['shape']);
        $subfile = 'heater.svg';
    }

    $svg = simplexml_load_file( $folder . $subfile);

    $paths = $svg->xpath('//svg:path');
    $tempBody = '';
    foreach ($paths as $path) {
        $tempBody .= $path->asXML();
    }
    $special['body'] = $tempBody;

    $paths = $svg->xpath('//svg:g[@id="body2"]/*');
    if (is_array($paths)) {
        $tempBody = '';
        foreach ($paths as $path) {
            $tempBody .= $path->asXML();
        }
        $special['body2'] = $tempBody;
    }

    switch ($options['shape']) {
        case 'drawshield':
            break;
        case 'french':
            break;
        case 'italian':
            break;
        case 'polish':
            break;
        case 'spanish':
            break;
        case 'sca':
            break;
        case 'english':
            break;
        case 'oval':
            break;
        case 'german':
            break;
        case 'square':
            break;
        case 'circle':
            break;
        case 'lozenge':
            break;
        case 'pauldron':
            $options['notrim'] = true; // don't put the grey translucent border on
            break;
        case 'swiss':
            break;
        default:
            break;
    }

    $special['chargesWithin'] = array ('w',chiefPresent() ? '120,420,760,560' : '120,120,760,860');

// only really applies to drawshield
    $special['chargesOn'] = chiefPresent() ? 'x///////////500,330,55,55:250,330:750,330:500,1150:35,700:965,700:35,450:965,450:50,900:950,900:200,1050:800,1047' : 'x///////////500,30,55,55:250,30:750,30:500,1150:35,500:965,500:35,200:965,200:35,800:965,800:200,1050:800,1050';

    return ($special);

}

function makeOrdinary( DOMElement &$node ) {
  /**
   * @var
   */
  global $options, $messages;

	// Get information about the shapes to be drawn
	// We do this first in case the ordinary code changes any of the modifiers, e.g. a bend quarterly
    $subtype = $node->getAttribute('keyterm');
    // special handling for bordures (as lots of options)
    if ($subtype == 'bordure') {
        $ord_data = getSpecial('bordures', $node);
    } elseif ($subtype == 'orle') {
        $ord_data = getSpecial('orles', $node);
    } else {
        $ord_data = includeOrdinary( $node );
    }
  /**
   * @var DOMElement $rev
   */
  if ( $rev = getModifierNodeWithKeyterm($node,'reversed',true)) // reversed means inverted for ordinaries
    $rev->setAttribute('keyterm','inverted');

  // On X another Y means "voided", so we fix it here
  // Note, both $on_ord and $on_object might be used later on
  if ( (($on_ord = getModifierNodeWithKeyterm($node,'on')) != null) or (($on_ord = getModifierNodeWithKeyterm($node,'charged-with')) != null)) {
    $onObject = $on_ord->firstChild;
    if ( $onObject->getAttribute('keyterm') == 'another') {
      $voidTincture = getTinctureNode($onObject);
      $myVoided = createModifierNode('ordmods', 'voided');
      $myVoided->appendChild($voidTincture);
      $node->replaceChild($myVoided,$on_ord);
      $on_ord = null;
    }
  }

	// Some defaults (either from the ordinary data or from modifiers)
  if ( array_key_exists('feature_size', $ord_data) ) { 
    $feature_size = $ord_data['feature_size'];
  } elseif ($options['shape'] == 'flag') {
    $feature_size = (int)($options['flagHeight'] * 0.05);
  } else {
	  $feature_size = 40;
  }
  /**
   * @var DOMElement $voided
   */
  $voided = getModifierNodeWithKeyterm($node, 'voided');
	$tinc1 = getTinctureNode($node,'1');
	$cotiseSVG = '';
	
	// Create the basic shape
	$bodySVG = '';
	if ( array_key_exists('shape_spec', $ord_data) ) { //  we are given a path specification to convert to SVG
	  $shape_spec = choose_fit($ord_data['shape_spec']);
    if ( $voided != null ) {
      if ( ($void_col = $voided->firstChild ) != null )
			  $bodySVG .= apply_tincture ( $void_col, createBody($shape_spec, 'none', 0 ) );
      $bodySVG .= createVoid($shape_spec, getModifierByType($node,'linetype', false), $feature_size);
    } else {
      $bodySVG .= createBody($shape_spec, getModifierByType($node,'linetype', false), $feature_size);
    }
		// Now do the cotises
    /**
     * @var DOMElement $cot_node
     */
    if ( ($cot_node = getModifierNodeWithKeyterm($node,'cotticing', false)) != null ) { // Need the value later
			$cot_linetype= getModifierByType($cot_node,'linetype', false);
			$cot_tinc = getTinctureNode($cot_node);
			if ( !$cot_tinc )
			  $cot_tinc = $tinc1;
      $numberCottices = $cot_node->getAttribute('value');
      for ( $i = 1; $i <= $numberCottices; $i++ )
			  $cotiseSVG .= createCotise( $shape_spec, $i, $cot_linetype );
		  $cotiseSVG = apply_tincture ( $cot_tinc, $cotiseSVG );
      existModifierWithKeyterm($node,'cotticing'); // mark as used
		}
  } else { // we are given direct SVG data
    if ( $voided != null ) {
		  if ( !array_key_exists('voided', $ord_data) ) {
        $messages->addMessage('warning','cannot draw ' . $subtype . ' as voided' . getLine($node));
        $bodySVG .= subPercent(choose_fit($ord_data['body']));
		  } else {
        if ( ($void_col = $voided->firstChild ) != null ) 
			    $bodySVG .= apply_tincture ( $void_col, subPercent(choose_fit($ord_data['body'])) );
			  $bodySVG .= subPercent(choose_fit($ord_data['voided']));
			}
    } else {
      $bodySVG .= subPercent(choose_fit($ord_data['body']));
    }
	} // We now have $body and $cotise set.
	
	// Find out how we are arranged
  if ( array_key_exists('rotate_pattern', $ord_data) && $tinc1->firstChild->nodeName != blazonML::E_COUNTERCHANGED )
	  $rot = $ord_data['rotate_pattern'];
	else 
	  $rot = 0;
	$mirrored = existModifierWithKeyterm($node, 'sinister') || $subtype == 'scarpe';
	$inverted = existModifierWithKeyterm($node, 'inverted');

	// Start to build return string
  $retval = apply_tincture ( $tinc1, $bodySVG, '', $rot, $mirrored, $inverted );

	// Do we need to add a second tincture as an overlay
  if ( ($compNode = getModifierNodeWithKeyterm($node,'compony1', false)) != null or
    ($compNode = getModifierNodeWithKeyterm($node, 'compony2', false )) != null ) {
    if ( ($body2_col = getTinctureNode($compNode)) == null or
      !array_key_exists('body2', $ord_data)) {
        $messages->addMessage('warning','Cannot apply compony to ' . $subtype);
    } else {
      $body2_spec = subPercent(choose_fit($ord_data['body2']));
      if ( $body2_spec{0} != '<' ) {
        $linetype = null;
        // Is there a linetype here also
        if ( ($div2ord = getModifierNodeWithKeyterm($node,'div2ord')) != null )
          $linetype = getModifierByType($node,'linetype', false);
        $body2_spec = '<path d="' . makePath($body2_spec, $linetype, 30) . '" />';
      }
      $body2 = apply_tincture ( $body2_col, $body2_spec, '', $rot, $mirrored, $inverted );
      $body2_fill = add_def( 'pattern patternContentUnits="userSpaceOnUse" patternUnits="userSpaceOnUse" x="0" ' .
                             'y="0" width="1000" height="1200"', $body2 );
      $retval .= add_def() . '<g fill="url(#' . $body2_fill . ')">' . $bodySVG . '</g>';
    }
  }
	
	// Fimbriation goes on top of everything else
	if ( ($fimbriated = getModifierNodeWithKeyterm($node,'fimbriated')) != null ) {
		$fim_rgb = get_rgb( getTinctureNode($fimbriated) );
	  $retval .= '<g stroke="' . $fim_rgb . '" stroke-width="15" fill="none">' . "$bodySVG</g>";
	}
  if ( existModifierWithKeyterm($node,'depressed') )
	  $retval .= '<g stroke="#000000" stroke-width="7">' . "$bodySVG</g>";
  if ( array_key_exists('body3',$ord_data) ) { // Other embellishments
    $retval .= subPercent($ord_data['body3']);
  }
		

  // Now add anything that is "on" the ordinary
	$onObjectSVG = '';
  /**
   * @var DOMElement $on_ord
   */
  if ( $on_ord != null ) {
    /**
     * @var DOMElement $onObject
     */
    switch ( $onObject->nodeName ) {
      case 'ordinary':
			  // If the underlying ordinary is mirrored, we need to mirror the "on" item so it ends up back the right way
				if ( $mirrored ) {
  				if ( ($sin = getModifierNodeWithKeyterm($onObject,'sinister')) != null )
					  $onObject->removeChild($sin);
					else
					  $onObject->appendChild(createModifierNode('chargemod','sinister'));
				}
			  // Make a normal ordinary, ignore any chief present
        chief('push','NC');
        $new_ord = makeOrdinary($onObject);
        chief('pop');
				// Does the main ordinary want this handled specially (scaling and clipping to fit entirely)?
			if ( array_key_exists('if_ord_on',$ord_data) and $ord_data['if_ord_on'] == 'scale' ) {
        $type = $onObject->getAttribute('keyterm'); // TODO can we find this out from a flag?
			  $height = ($type == 'per-bend' || $type == 'per-bend-sinister' || $type == 'quarterly') ? 1000 : 1200;
			  $patt_id = add_def ( 'pattern width="100%" height="100%" patternUnits="objectBoundingBox" preserveAspectRatio="none" viewBox="0 0 1000 ' .   $height . '"',
               "<desc>Ordinary on ordinary</desc>" . $new_ord );
          $onObjectSVG = '<g fill="url(#' . $patt_id . ')"  >' . $bodySVG . '</g>';
			  } else {
          $fill = add_def( 'pattern patternContentUnits="userSpaceOnUse" patternUnits="userSpaceOnUse" x="0" ' .
                             'y="0" width="1000" height="1200"', $new_ord );
					// TODO should this be a mask? Or a clip path?
          $onObjectSVG = '<g fill="url(#' . $fill . ')">' . $bodySVG . '</g>';
			  } 
        break;
      case 'charge':
 			  // If the underlying ordinary is mirrored, we need to mirror the "on" item so it ends up back the right way
				if ( $mirrored ) {
  				if ( ($sin = getModifierNodeWithKeyterm($onObject,'reversed')) != null )
					  $onObject->removeChild($sin);
					else
					  $onObject->appendChild(createModifierNode('chargemod','reversed'));
				}
        if ( array_key_exists('chargesOn', $ord_data)) {
          $onPlacement = $ord_data['chargesOn'];
          if ( is_array($onPlacement )) { // Means we have to call calcPlace
            $onPlacement = calcPlace($onObject,$onPlacement[0],subPercent($onPlacement[1]));
          } else {
            $onPlacement = subPercent($onPlacement);
          }
        } else {
          $onPlacement = 'n'; // error if no placement for ordinary with charges on it, but let placeCharges handle it
        }
        $onObjectSVG = makeCharge( $onObject, $onPlacement );
        break;
    }
  }
  
  // Is this ordinary "between" anything?
  $chargeWithinSVG = '';
  /**
   * @var DOMElement $btwn_ord
   */
  foreach ( getModifiersByTypeAsArray($node,'between',true) as $chargesWithin) {
    $chargesWithin = (object)$chargesWithin->firstChild;
    // If the underlying ordinary is mirrored, we need to mirror the "on" item so it ends up back the right way
    if ($mirrored) {
      if (($sin = getModifierNodeWithKeyterm($chargesWithin, 'reversed')) != null)
        $chargesWithin->removeChild($sin);
      else
        $chargesWithin->appendChild(createModifierNode('chargemod', 'reversed'));
    }
    if (array_key_exists('chargesWithin', $ord_data)) {
      $withinPlacement = $ord_data['chargesWithin'];
      if (is_array($withinPlacement)) { // Means we have to call calcPlace
        $withinPlacement = calcPlace($chargesWithin, $withinPlacement[0], $withinPlacement[1]);
      }
    } else {
      $withinPlacement = 'n'; // error if no placement for ordinary with charges on it, but let placeCharges handle it
    }
    $chargeWithinSVG = makeCharge($chargesWithin, $withinPlacement);
  }
  	
	// Gather all the parts together
  $retval = add_def() . $retval . $cotiseSVG . $onObjectSVG . $chargeWithinSVG;
	
	// Are there any modifiers that affect over all position?
	$transform = '';
  if ( $mirrored ) {
    $transform .= 'translate(1000,0) scale(-1,1) ';
  }
  if ( $inverted ) {
    $transform .= 'translate(0,1200) scale(1,-1) ';
  }
  if ( existModifierWithKeyterm($node,'enhanced') ) {
    $yshift = -200; // Adjust to change movement for enhanced (match in layout.inc)
    $transform .= 'translate(0,' . $yshift . ') ';
  }
  if ( existModifierWithKeyterm($node,'abased' )) {
    $yshift = 200; // As above
    $transform .= 'translate(0, ' . $yshift . ') ';
  }
  if ( $transform != '' )
    $retval = '<g transform="' . $transform . '">' . $retval . "</g>\n";
	
	// And finally...
  $retval = '<g><title>' . $subtype . '</title>' . $retval . '</g>';

  if ( $options['palette'] == 'outline' )
    $retval = '<g stroke-width="3" stroke="' . rgb('ordinary-stroke') . '" >' . $retval . '</g>';

	return $retval;
}

