
// rahad 1183
import java.util.Scanner;

public class mylab4{
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.println("Enter your name and age :");
        String name = scanner.nextLine();
        int age = scanner.nextInt();

        System.out.print("Hello, " + name );
        System.out.println("! You are " + age + " years old.");
    }
}





problem 2 answer:





import java.util.Scanner;

public class mylab5 {
    public static void main(String[] args) {
        Scanner Nazisra = new Scanner(System.in);

        System.out.println("Enter a number:");
        int number = Nazisra.nextInt();

        if (number > 0) {
            System.out.println(number + " is a positive number.");
        } else if (number < 0) {
            System.out.println(number + " is a negative number.");
        } else {
            System.out.println(number + " is zero.");
        }
    }
}








