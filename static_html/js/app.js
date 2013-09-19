angular.module('batyrd', []).
    config(['$routeProvider', function($routeProvider) {
        $routeProvider.
        when('/status', {templateUrl: 'partials/status.html', controller: StatusCtrl}).
        when('/about', {templateUrl: 'partials/about.html', controller: AboutCtrl}).
        when('/jobqueue', {templateUrl: 'partials/jobqueue.html', controller: JobsCtrl}).
        when('/layers', {templateUrl: 'partials/layers.html', controller: LayersCtrl}).
        otherwise({redirectTo: '/status'});
    }]).
    filter('fromNow', function() {
        return function(date) {
            return moment(date).fromNow();
        }
    });
