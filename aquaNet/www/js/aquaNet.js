$(document).ready(function() {
  $('body').append($('<div class="container-fluid" id="container"><h1>Aquanet</h1><div id="main"><div id="module-list" class="row"></div></div></div>')) ;

  var AquaNetModule = Backbone.Model.extend({
    defaults: function() {
      var model = {
        name: "",     // Name of the module
        type: "master", // master or station
        localIP: "",  // IP on the local domestic wifi network
        APName: "",   // Name of the created wifi network
        APIP: "",     // IP on the Access Point created wifi network
        temp: 0,
        tempAlert: "no",
        minTemp: 0,
        maxTemp: 0,
        maxAdj: 0,
        light: 0,
        lightAlert: "no",
        "minOnLight": 0,
        "maxOnLight": 0,
        "minOffLight": 0,
        "maxOffLight": 0,
        waterLevel: "high",
        power: "on",         // off, on
        oneAlert: "no",
        date: ""
      };
      return model;
    },

    // Server returns some numeric values, not suited to use as class names
    parse: function(data) {

      data.oneAlert = data.oneAlert ? "isalert" : "noalert";
      data.power = data.powerAlert ? "off" : "on";
      data.waterLevel = data.waterLevelAlert ? "low": "high" ;
      data.lightAlert = data.lightAlert ? "isalert" : "noalert";
      data.tempAlert = data.tempAlert ? "isalert" : "noalert";
      data.type = data.type ? "master" : "station";
      data.name = data.name || "Aquarium";
      data.temp = data.temp/100;
      data.minTemp = data.minTemp/100;
      data.maxTemp = data.maxTemp/100;
      data.tempAdj = data.tempAdj/100;
      return data;
    }
  });

  var url = document.location.href.split('/');
  url.pop();
  url = url.join('/') + "/getData.json"
  var AquaNetList = Backbone.Collection.extend({
    model: AquaNetModule,
    url: url
  });

  var modules = new AquaNetList;

  var AquaNetView = Backbone.View.extend({
    tagName: "div",
    template: _.template('\
<div class="module <%- type %> <%- oneAlert %>">\
  <div class="name <%- name %>"><%- name %><span class="icon-cog"/></div>\
  <div class="localIP"><%- localIP %></div>\
  <div class="APName"><%- APName %></div>\
  <div class="APIP"><span><%- APName %></span><%- APIP %></div>\
  <div class="temperature <%- tempAlert %>"><%- temp %></div>\
  <div data="temperatureRange" class="setting temperatureRange"><%- minTemp %> - <%- maxTemp %><span class="icon-pencil"/>\
    <div class="editor temperatureRange"><input class="minTemp" value="<%- minTemp %>"/><input class="maxTemp" value="<%- maxTemp %>"/><button class="save"><l/></button><span class="icon-cancel-circle"/></div>\
  </div>\
  <div data="temperatureAdjustment" class="setting temperatureAdjustment"><%- tempAdj %><span class="icon-pencil"/>\
    <div class="editor temperatureAdjustment"><input class="tempAdj" value="<%- tempAdj %>"/><button class="save"><l/></button><span class="icon-cancel-circle"/></div>\
  </div>\
  <div class="light <%- lightAlert %>"><%- light %></div>\
  <div data="onLightRange" class="setting onLightRange"><%- minOnLight %> - <%- maxOnLight %><span class="icon-pencil"/>\
    <div class="editor onLightRange"><input class="minOnLight" value="<%- minOnLight %>"/><input class="maxOnLight" value="<%- maxOnLight %>"/><button class="save"><l/></button><span class="icon-cancel-circle"/></div>\
  </div>\
  <div data="offLightRange" class="setting offLightRange"><%- minOffLight %> - <%- maxOffLight %><span class="icon-pencil"/>\
    <div class="editor offLightRange"><input class="minOffLight" value="<%- minOffLight %>"/><input class="maxOffLight" value="<%- maxOffLight %>"/><button class="save"><l/></button><span class="icon-cancel-circle"/></div>\
  </div>\
  <div class="waterLevel <%- waterLevel %>"></div>\
  <div class="power <%- power %>"></div>\
  <div class="date"><%- date %></div>\
</div>'),
    initialize: function() {
      this.listenTo(this.model, 'change', this.render);
    },
    events: {
      "click button": "saveData",
      "click .icon-pencil": "openEditor",
      "click .icon-cancel-circle": "closeEditors",
      "click .icon-cog": "toggleSettings",
      "keyup .module": "keyUp"
    },
    render: function() {
      this.$el.html(this.template(this.model.toJSON()));
      this.$el.addClass("col-xs-12 col-sm-4 col-md-4 col-lg-3") ;
      return this;
    },
    keyUp: function(e) {
      if(e.keyCode == 27) {
        this.closeEditors();
      }
    },
    openEditor: function(e) {
      $('.editor').hide();
      var data = $(e.currentTarget).parents("div.setting").attr("data");
      $('.editor.' + data).show();
      $('.editor.' + data + ' input').first().focus().select();
      e.stopPropagation();
    },
    toggleSettings: function() {
      this.closeEditors();
      $("body").toggleClass("showSettings");
    },
    closeEditors: function(e) {
      $('.editor').hide();
    },
    saveData: function(e) {
      this.closeEditors();
      var message = "";
      var data = $(e.target).parents("div.setting").attr("data");
      var url = document.location.href.split('/');
      url.pop();
      url = url.join('/') + "/msgArduino";
      var params = {};
      var send = false;
      // TODO: need to use localized message... or new "technical" messages not localized ? => modif arduino code, add many messages... :(
      switch(data) {
        case "onLightRange":
          params.command = "light limits on: " + $('input.minOnLight').val() + '-' + $('input.maxOnLight').val();
          send = true;
          break;
        case "offLightRange":
          params.command = "light limits off: " + $('input.minOffLight').val() + '-' + $('input.maxOffLight').val();
          send = true;
          break;
        case "temperatureRange":
          params.command = "temp " + parseFloat($('input.minTemp').val())*100 + ' ' + parseFloat($('input.maxTemp').val())*100;
          send = true;
          break;
        case "temperatureAdjustment":
          params.command = "temp adj " + parseFloat($('input.tempAdj').val()*100);
          send = true;
          break;
      }
      if(send) {
        $.ajax(url, {
          complete: function() {
            fetch();
          },
          data: params
        });
      }
    }
  });

  var AppView = Backbone.View.extend({
    el: $("#main"),
    initialize: function() {
      this.listenTo(modules, 'add', this.addOne);
      this.listenTo(modules, 'reset', this.addAll);
      this.listenTo(modules, 'all', this.render);

    },
    addOne: function(aModule) {
      var view = new AquaNetView({model: aModule});
      this.$("#module-list").append(view.render().el);
    },
    addAll: function() {
      this.$("#module-list").empty();
      modules.each(this.addOne, this);
    }
  });

  var app = new AppView;
  //setInterval(fetch, 10000);
  fetch();

  function fetch() {
    modules.fetch({
      reset: true,
      error: function(collection, response, options) {
        debugger;
      }
    });
  }

});

