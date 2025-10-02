package com.example.cylinderworks.engine

import java.util.HashMap
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.withLock

object EngineDiagnosticsRegistry {

    interface Provider {
        fun diagnostics(): Map<String, Any?>?
    }

    private val lock = ReentrantLock()
    private var provider: Provider? = null

    fun register(provider: Provider) {
        lock.withLock {
            this.provider = provider
        }
    }

    fun unregister(provider: Provider) {
        lock.withLock {
            if (this.provider == provider) {
                this.provider = null
            }
        }
    }

    fun snapshot(): MutableMap<String, Any?>? {
        val currentProvider = lock.withLock { provider }
        val data = currentProvider?.diagnostics() ?: return null
        return if (data is MutableMap<String, Any?>) {
            data
        } else {
            HashMap(data)
        }
    }
}
