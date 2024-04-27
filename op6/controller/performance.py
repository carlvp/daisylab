class PerformanceController:
    '''
    The PerformanceController manages the UI of the PerformanceScreen
    and mediates user interaction and operations in the performace mode
    '''

    def __init__(self):
        self.performanceScreen=None

    def registerControllerObjects(self, controllers):
        '''adds controller objects to the dictionary, controllers.'''
        controllers['PerformanceController']=self

    def setViews(self, views):
        '''connects to relevant views in the dictionary, views'''
        self.performanceScreen=views['PerformanceScreen']
