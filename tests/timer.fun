fun main() {
    # Sets the num of seconds to set the timer for
    long timer = 0

    while (timer < 6) {
        print (5 - timer)
        delay(1000000)
        timer = timer++
    }

    while (1) {
        bell
    }
}
