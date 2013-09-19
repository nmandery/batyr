/**
 * items to list in the menu
 */
function MenuListCtrl($scope) {
    $scope.menuitems = [
    {
        "name": "Status",
        "link": "status"
    },
    {
        "name": "Layers",
        "link": "layers"
    },
    {
        "name": "Jobqueue",
        "link": "jobqueue"
    }
    ];
};


function StatusCtrl($scope, $http, $timeout) {

    // update interval of in milliseconds
    $scope.updateIntervalMs = 5000;

    var fetchStatus = function() {
        $http.get('api/status.json').success(function(data) {
            $scope.status = data;
        });
    }
    fetchStatus();

    // periodic reload of the views data
    // this is ugly and the stack grows and grows
    // -- replace with something better
    var cancelRefresh;
    var intervalFetch = function() {
        cancelRefresh = $timeout(function performFetch() {
            fetchStatus()
            cancelRefresh = $timeout(intervalFetch, $scope.updateIntervalMs);
        }, $scope.updateIntervalMs);
    };
    intervalFetch();

    // destroy the timeout when the view is detroyed
    $scope.$on('$destroy', function(e) {
                $timeout.cancel(cancelRefresh);
    });
}

function AboutCtrl($scope, $http) {
    var fetchStatus = function() {
        $http.get('api/status.json').success(function(data) {
            $scope.status = data;
        });
    }
    fetchStatus();
}

function LayersCtrl($scope, $http) {
}

function JobsCtrl($scope, $http) {
    var fetchJobs = function() {
        $http.get('api/jobs.json').success(function(data) {
            $scope.jobs = data;
        });
    }
    fetchJobs();

    // refresh button
    $scope.refreshJobs = function() {
        fetchJobs();
    };
}

