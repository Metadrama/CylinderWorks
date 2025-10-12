package com.example.cylinderworks.engine

import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.withLock

object EngineControlRegistry {
    interface Controller {
        fun setTestRpm(rpm: Float)
    }

    private val lock = ReentrantLock()
    private var controller: Controller? = null

    fun register(controller: Controller) {
        lock.withLock {
            this.controller = controller
        }
    }

    fun unregister(controller: Controller) {
        lock.withLock {
            if (this.controller == controller) {
                this.controller = null
            }
        }
    }

    fun setTestRpm(rpm: Float) {
        val target = lock.withLock { controller }
        target?.setTestRpm(rpm)
    }
}
