import UIKit

class ViewController: UIViewController {
    private var metalLayer: CAMetalLayer!
    private var rendererBundle: RendererBundle?

    override func viewDidLoad() {
        super.viewDidLoad()
        metalLayer = CAMetalLayer()
        metalLayer.frame = view.frame
        view.layer.addSublayer(metalLayer)
    }

    private func recreateRenderer() {
        destroyRenderer()
                
        metalLayer.contentsScale = view.window!.screen.nativeScale
        metalLayer.drawableSize = CGSizeMake(metalLayer.frame.size.width * metalLayer.contentsScale,
                                             metalLayer.frame.size.height * metalLayer.contentsScale)
        rendererBundle = RendererBundle(metalLayer: metalLayer,
                                        width: Int32(metalLayer.frame.size.width * metalLayer.contentsScale),
                                        height: Int32(metalLayer.frame.size.height * metalLayer.contentsScale),
                                        interfaceSelectionIP: "127.0.0.1",
                                        daemonIP: "127.0.0.1")
        rendererBundle?.connect()
        rendererBundle?.run()
    }

    private func destroyRenderer() {
        rendererBundle = nil
    }
    
    override func viewWillTransition(to size: CGSize, with coordinator: UIViewControllerTransitionCoordinator) {
        coordinator.animate(alongsideTransition: nil) { context in
            self.metalLayer.frame = self.view.layer.frame
            self.recreateRenderer()
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        recreateRenderer()
    }

    override func viewDidDisappear(_ animated: Bool) {
        destroyRenderer()
    }
}
