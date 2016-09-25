$(document).ready(function() {
  $('body').append($('<div class="container-fluid" id="container"><h1>Aquanet</h1><div id="main"><div id="module-list" class="row"></div></div></div>')) ;

  var AquaNetModule = Backbone.Model.extend({
    defaults: function() {
      var model = {
        id: "",     // Name of the module
        type: "master", // master or station
        localIP: "",  // IP on the local domestic wifi network
        APName: "",   // Name of the created wifi network
        APIP: "",     // IP on the Access Point created wifi network
        temp: 0,
        tempAlert: "no",
        minTemp: 0,
        maxTemp: 0,
        tempAdj: 0,
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
  window.modules = modules;

  var AquaNetView = Backbone.View.extend({
    tagName: "div",
    template: _.template('\
<div class="module <%- type %> <%- oneAlert %>" moduleId="<%- id %>">\
  <div class="name"><%- id %><div class="commands"><span class="icon icon-floppy-disk"/><span class="icon icon-cog"/><span class="icon icon-loop2"/></div></div>\
  <div class="localIP"><%- localIP %></div>\
  <div class="APName"><%- APName %></div>\
  <div class="APIP"><span><%- APName %></span><%- APIP %></div>\
  <div class="temperature <%- tempAlert %>"><%- temp %></div>\
  <div data="temperatureRange" class="setting temperatureRange"><%- minTemp %> - <%- maxTemp %><span class="icon icon-pencil"/>\
    <div class="editor temperatureRange"><input class="minTemp" value="<%- minTemp %>"/><input class="maxTemp" value="<%- maxTemp %>"/><button class="save"><l/></button><span class="icon icon-cancel-circle"/></div>\
  </div>\
  <div data="temperatureAdjustment" class="setting temperatureAdjustment"><%- tempAdj %><span class="icon icon-pencil"/>\
    <div class="editor temperatureAdjustment"><input class="tempAdj" value="<%- tempAdj %>"/><button class="save"><l/></button><span class="icon icon-cancel-circle"/></div>\
  </div>\
  <div class="light <%- lightAlert %>"><%- light %></div>\
  <div data="onLightRange" class="setting onLightRange"><%- minOnLight %> - <%- maxOnLight %><span class="icon icon-pencil"/>\
    <div class="editor onLightRange"><input class="minOnLight" value="<%- minOnLight %>"/><input class="maxOnLight" value="<%- maxOnLight %>"/><button class="save"><l/></button><span class="icon icon-cancel-circle"/></div>\
  </div>\
  <div data="offLightRange" class="setting offLightRange"><%- minOffLight %> - <%- maxOffLight %><span class="icon icon-pencil"/>\
    <div class="editor offLightRange"><input class="minOffLight" value="<%- minOffLight %>"/><input class="maxOffLight" value="<%- maxOffLight %>"/><button class="save"><l/></button><span class="icon icon-cancel-circle"/></div>\
  </div>\
  <div class="waterLevel <%- waterLevel %>"></div>\
  <div class="power <%- power %>"></div>\
  <div data="date" class="setting date"><%- date %><span class="icon icon-pencil"/>\
    <div class="editor date"><input class="date" value="<%- date %>"/><button class="save"><l/></button><span class="icon icon-cancel-circle"/></div>\
</div>'),
    initialize: function() {
      this.listenTo(this.model, 'change', this.render);
    },
    events: {
      "click button": "saveSetting",
      "click .icon-pencil": "openEditor",
      "click .icon-cancel-circle": "closeEditors",
      "click .icon-cog": "toggleSettings",
      "click .icon-loop2": "refreshData",
      "click .icon-floppy-disk": "saveConfig",
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
      // when handling several modules, need to make sure we target the right one's DOM
      var parent = $(e.currentTarget).parents("div.setting").first();
      var data = parent.attr("data");
      parent.find('.editor.' + data).show();
      parent.find('.editor.' + data + ' input').first().focus().select();
      $('body').addClass("editing");
      e.stopPropagation();
    },
    toggleSettings: function() {
      this.closeEditors();
      $("body").toggleClass("showSettings");
    },
    closeEditors: function(e) {
      $('.editor').hide();
      $('body').removeClass("editing");
    },
    refreshData: function() {
      modules.fetch({
        reset: true,
        error: function(collection, response, options) {
          debugger;
        }
      });
    },
    saveConfig: function() {
      this.send({command: "save"});
    },

    saveSetting: function(e) {
      this.closeEditors();
      var message = "";
      // when handling several modules, need to make sure we target the right one's DOM
      var parent = $(e.target).parents("div.setting").first();
      var data = parent.attr("data");
      var params = {};
      var send = false;
      // TODO: need to use localized message... or new "technical" messages not localized ? => modif arduino code, add many messages... :(
      switch(data) {
        case "onLightRange":
          var min = parent.find('input.minOnLight').val();
          var max = parent.find('input.maxOnLight').val();
          params.command = "light limits on: " + min  + '-' + max;
          // update model
          this.model.set('minOnLight', parseInt(min));
          this.model.set('maxOnLight', parseInt(max));
          send = true;
          break;
        case "offLightRange":
          var min = parent.find('input.minOffLight').val();
          var max = parent.find('input.maxOffLight').val();
          params.command = "light limits off: " + min  + '-' + max;
          // update model
          this.model.set('minOffLight', parseInt(min));
          this.model.set('maxOffLight', parseInt(max));
          send = true;
          break;
        case "temperatureRange":
          var min = parseFloat(parent.find('input.minTemp').val());
          var max = parseFloat(parent.find('input.maxTemp').val());
          params.command = "temp " + min * 100 + ' ' + max * 100;
          this.model.set('minTemp', min);
          this.model.set('maxTemp', max);
          send = true;
          break;
        case "temperatureAdjustment":
          var tempAdj = parseFloat(parent.find('input.tempAdj').val());
          params.command = "temp adj " + tempAdj * 100;
          this.model.set('tempAdj', tempAdj);
          send = true;
          break;
        case "date":
          var date = parent.find('input.date').val();
          params.command = "time " + date;
          this.model.set('date', date);
          send = true;
          break;
      }
      if(send) {
        this.send(params);
      }
    },

    send: function(data) {
      var url = document.location.href.split('/');
      url.pop();
      url = url.join('/') + "/msgArduino";
      $.ajax(url, {
        data: data
      });
    }
  });

  var AppView = Backbone.View.extend({
    el: $("#main"),
    initialize: function() {
      this.listenTo(modules, 'add', this.addOne);
      this.listenTo(modules, 'reset', this.addAll);
    },

    addOne: function(aModule) {
      var view = new AquaNetView({model: aModule, id: aModule.id});
      this.$("#module-list").append(view.render().el);
    },
    addAll: function() {
      this.$("#module-list").empty();
      modules.each(this.addOne, this);
    }
  });

  var app = new AppView({model: modules, id: "modules"});
  window.app = app;
  setInterval(fetch, 10000);
  fetch();

  function fetch() {
    if(!$('body').hasClass("editing")) {
      app.model.fetch({
        reset: false,
        error: function(collection, response, options) {
          debugger;
        }
      });
    }
  }

});

