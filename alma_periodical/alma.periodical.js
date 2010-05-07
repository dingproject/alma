/* 
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
Drupal.behaviors.almaPeriodical = function (context) {
  var years = $('.year');
  if (years.length > 1) {
    // Hide all an click event
    years.each( function () {
      $('ul', this).hide();
      $(this).toggle(function () {
        $('ul', this).show();
        $(this).addClass("expanded");
      }, function() {
        $('ul', this).hide();
        $(this).removeClass("expanded")
      })
      .css('cursor', 'pointer');
    });
  }

};

Drupal.almaPeriodical = function (context) {
  
};