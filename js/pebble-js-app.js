


var inicializado = false;
var configColor = localStorage.getItem('config_color') || 0;
var configShowDate = localStorage.getItem('config_showdate') || 0;

Pebble.addEventListener("ready", function() {
  console.log("ready to rumble!");
  inicializado = true;

});

Pebble.addEventListener("showConfiguration", function() {
  console.log("Cargando Interface Configuracion");

  var url = 'http://dl.dropboxusercontent.com/u/50626058/config.html' + '?pColor=' + configColor + '&pFecha=' + configShowDate;
  Pebble.openURL(url);

});

Pebble.addEventListener("webviewclosed", function(e) {
  
  console.log("configuration closed");
  // webview closed

  if(e.response){

    // recibir mensaje en formato JSON
    var options = JSON.parse(decodeURIComponent(e.response));

    // obtener valores de configuracion
    configColor = options['config-color'];
    configShowDate = options['config-showdate'];
    
    // armar mensaje
    var configParams = { 'color': configColor, 'date': configShowDate };

    // mandar mensaje a Pebble
    var transactionId = Pebble.sendAppMessage(configParams,
  	 function(e) {
    	console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
  		},
  	function(e) {
    	console.log("Unable to deliver message with transactionId=" + e.data.transactionId );
  		}
	 );

    // almacenar para uso posterior
    localStorage.setItem('config_showdate', configShowDate);
    localStorage.setItem('config_color', configColor);

    console.log("Options = " + JSON.stringify(options));
  }
  else
    console.log("No configuration data received.");
    
	});
