$(document).ready(function() {
  updateDiv();
  // Auto-refresh every second
  setInterval(updateDiv, 1000);
});

// update the content of div elements
function updateDiv() {
  $.getJSON("json", function() {
    //alert( "success" );
  })
    .done(function(data) {
      console.log(data);
      $.each(data, function(key, value) {
        // Skip dmx array for standard element update
        if (key === 'dmx') {
          updateDmxGrid(value);
          return;
        }
        
        elem = document.getElementById(key);
        if (elem)
          elem.innerHTML = value;
      });
      //alert( "done" );
    })
    .fail(function() {
      //alert( "error" );
    })
    .always(function() {
      //alert( "finished" );
    });
}

// Update DMX channel grid
function updateDmxGrid(dmxData) {
  if (!dmxData || dmxData.length === 0) return;
  
  var grid = $('#dmx-grid');
  grid.empty();
  
  for (var i = 0; i < dmxData.length; i++) {
    var channel = $('<div class="dmx-channel"></div>');
    channel.append('<div class="ch-num">' + (i + 1) + '</div>');
    channel.append('<div class="ch-val">' + dmxData[i] + '</div>');
    
    // Color code based on value
    var brightness = Math.floor((dmxData[i] / 255) * 200 + 55);
    channel.css('background-color', 'rgb(' + brightness + ',' + brightness + ',' + brightness + ')');
    
    grid.append(channel);
  }
}
