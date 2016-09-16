
$(document).ready(function() {
  initDom();
  init();
});

function initDom() {
  $('body').append($('<h1>Aquanet</h1>')) ;
  $('body').append($('<form method="post" action="/msgArduino">')) ;
  $('body').append($('<label>Arduino command: <input type="text" name="cmd"/></label>')) ;
  $('body').append($('<button type="submit">Send</button>')) ;
  $('body').append($('</form>')) ;
  $('body').append($('<form method="post" action="/msgESP">')) ;
  $('body').append($('<label>ESP command: <input type="text" name="cmd"/></label>')) ;
  $('body').append($('<button type="submit">Send</button>')) ;
  $('body').append($('</form>')) ;
  $('body').append($('<div id="status"></div>')) ;
}

function init() {
  setInterval(checkStatus, 5000);
}

function checkStatus() {
  $.ajax({
    url: "/method?name=getStatus",
    onSuccess: function(data) {
      $('#status').text(data);
    }
  })
}