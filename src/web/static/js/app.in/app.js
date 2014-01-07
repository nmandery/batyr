angular.module('batyrd', ['ngRoute', 'cgNotify']).
    config(['$routeProvider', function($routeProvider) {
        $routeProvider.
        when('/status', {templateUrl: 'partials/status.html', controller: StatusCtrl}).
        when('/manual', {templateUrl: 'partials/manual.html', controller: ManualCtrl}).
        when('/jobqueue', {templateUrl: 'partials/jobqueue.html', controller: JobsCtrl}).
        when('/layers', {templateUrl: 'partials/layers.html', controller: LayersCtrl}).
        when('/job/:jobId', {templateUrl: 'partials/job.html', controller: JobCtrl}).
        otherwise({redirectTo: '/status'});
    }]).
    filter('fromNow', function() {
        // return a natural language string describing the
        // timestamp relative to the current time.
        // Input is expected to be a string in ISO-8601 format.
        return function(date) {
            if (!date) {
                return "";
            }
            return moment(date).fromNow();
        }
    }).
    filter('timestamp', function() {
        // return a natural language string describing the
        // timestamp
        // Input is expected to be a string in ISO-8601 format.
        return function(date) {
            if (!date) {
                return "";
            }
            return moment(date).format("dddd, MMMM Do YYYY, hh:mm:ss");
        }
    }).
    filter('secondDuration', function() {
        // display the duration as a human-compatible text. input is expected
        // to be in seconds
        return function(secs) {
            if (!secs) { // fallback, as angular initalizes the view with undefined,
                         // what moment.js does not especially like.
                return "? minutes";
            }
            return moment.duration(secs, 'seconds').humanize();
        }
    }).
    run(function($templateCache) {
        // the the template for notifications
        $templateCache.put("notification.html",
            "<div class=\"notification {{type}}\">" +
            "	{{message}}" +
            "</div>"
        )
    }).
    factory('NotificationService', ['notify', function (notify) {
        return {
            /** function to format notifications */
            notify: function(message, type) {
                var css = "notification-bg";
                if (type) {
                    css = type;
                }
                notify({
                    message: message,
                    template: "notification.html",
                    scope: {
                        type: css
                    }
                });
            }
        }
    }]);


/* vim: set ft=javascript */
