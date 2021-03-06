

/**
 * items to list in the menu
 */
function MenuListCtrl($scope) {
    $scope.menuitems = [
    {
        "name": "Status",
        "icon": "icon-info-sign",
        "link": "status"
    },
    {
        "name": "Layers",
        "icon": "icon-reorder",
        "link": "layers"
    },
    {
        "name": "Jobqueue",
        "icon": "icon-cogs",
        "link": "jobqueue"
    }
    ];
};


function StatusCtrl($scope, $http, $timeout) {

    // update interval of in seconds
    $scope.updateIntervalSecs = 5;

    var fetchStatus = function() {
        $http.get('api/v1/status.json').success(function(data) {
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
            cancelRefresh = $timeout(intervalFetch, $scope.updateIntervalSecs * 1000);
        }, $scope.updateIntervalSecs * 1000);
    };
    intervalFetch();

    // destroy the timeout when the view is detroyed
    $scope.$on('$destroy', function(e) {
                $timeout.cancel(cancelRefresh);
    });
}

function ManualCtrl($scope, $http) {
    var fetchStatus = function() {
        $http.get('api/v1/status.json').success(function(data) {
            $scope.status = data;
        });
    }
    fetchStatus();
}

function LayersCtrl($scope, $http, NotificationService) {
    $scope.showLayerHelp = false;
    $scope.layerlist = {};

    var fetchLayers = function() {
        // clear layerlist object to avoid displaying outdated data before
        // the request bellow is finished
        $scope.layerlist = {};

        $http.get('api/v1/layers.json').success(function(data) {
            $scope.layerlist = data;
        });
    }
    fetchLayers();

    $scope.toggleFilterHelp = function() {
        $scope.showLayerHelp = !$scope.showLayerHelp;
    };
    $scope.pullLayer = function(layer) {
        var job = {
            layerName:  layer.name,
            filter:     layer.filter
        };
        $http.post('api/v1/pull', job).then(
            // success
            function(resp) {
                NotificationService.notify(
                    "Layer " + resp.data.layerName + " added to queue",
                    resp.data.status);
            },
            // error
            function (res) {
                NotificationService.notify(
                    "Could not add layer " + layer.name + " to queue",
                    "failed");
            }
        )
    }
}

function JobsCtrl($scope, $http, $location) {
    $scope.joblist = {};
    var fetchJobs = function() {
        $http.get('api/v1/jobs.json').success(function(data) {
            $scope.joblist = data;
        });
    }
    fetchJobs();

    // refresh button
    $scope.refreshJobs = function() {
        fetchJobs();
    };

    $scope.showJob = function(job) {
        $location.path('/job/' + job.id);
    }
}

function JobCtrl($scope, $http, $routeParams, NotificationService) {
    $scope.job = {};
    var fetchJob = function() {
        $http.get('api/v1/job/' + $routeParams.jobId + '.json').success(
            function(data) {
                $scope.job = data;
            }).error(
            function(res) {
                NotificationService.notify("Job " + $routeParams.jobId + " does not exist (anymore).");
            }
        );
    }
    fetchJob();
}



/* vim: set ft=javascript */
