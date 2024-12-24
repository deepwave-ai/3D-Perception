import androidx.compose.foundation.layout.*
import androidx.compose.foundation.Image
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.res.painterResource
import kotlinx.coroutines.delay
import com.digisensing.esp32cameracontrol.R

@Composable
fun SplashScreen(onSplashTimeout: () -> Unit) {
    // Add a delay to simulate the splash screen duration
    LaunchedEffect(Unit) {
        delay(3000L) // 3 seconds
        onSplashTimeout()
    }

    // Content of the splash screen
    Surface(
        modifier = Modifier.fillMaxSize(),
        color = MaterialTheme.colorScheme.primary
    ) {
        Column(
            modifier = Modifier.fillMaxSize(),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            // Replace with your app's logo or branding
            Image(
                painter = painterResource(id = R.drawable.digisensing),
                contentDescription = "App Logo",
                modifier = Modifier.size(100.dp)
            )
            Spacer(modifier = Modifier.height(16.dp))
            Text(
                text = "Welcome",
                fontSize = 24.sp,
                color = MaterialTheme.colorScheme.onPrimary
            )
        }
    }
}
